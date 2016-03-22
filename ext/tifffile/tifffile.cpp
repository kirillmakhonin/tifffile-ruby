#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "tiffio.h"
#include <vector>
#include <algorithm>

using namespace std;

template <class T>
static bool rows_size_compare(std::vector<T> & a, std::vector<T> & b)
{
    return (a.size() < b.size());
}

template <class T>
void calculate_width_height(std::vector<std::vector<T> > & data, uint32 & width, uint32 & height){
    height = data.size();
    width = std::max_element(data.begin(), data.end(), rows_size_compare<T>)->size();
}


template <class T>
void write_data_to_row_buffer(std::vector<T> & data, unsigned char *buf, uint64 buff_size, uint8 sampleperpixel, uint8 bitspersample){
    memset(buf, 0, buff_size);
    buff_size = std::min<uint64>(buff_size, sizeof(T) * data.size());
    memcpy(buf, (void *)(&(data[0])), buff_size);
}


template <class T>
bool write_tiff_file(std::string filename, std::vector<std::vector<T> > & data, uint8 sampleperpixel, uint8 bitspersample, std::string description, std::string software){
    try {
        TIFF* tif = TIFFOpen(filename.c_str(), "w");

        uint32 width, height;
        calculate_width_height(data, width, height);

        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, (int)ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, (char*)description.c_str());
        TIFFSetField(tif, TIFFTAG_SOFTWARE, (char*)software.c_str());

        uint64 linebytes = sampleperpixel * width * (bitspersample / 8);
        unsigned char *buf = NULL;
        buf = (unsigned char *)_TIFFmalloc(linebytes);

        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

        for (uint32 row = 0; row < height; row++){
            std::vector<T> row_data = data[row];
            write_data_to_row_buffer(row_data, buf, linebytes, sampleperpixel, bitspersample);
            if (TIFFWriteScanline(tif, buf, row, 0) < 0)
                break;
        }

        if (buf)
            _TIFFfree(buf);

        TIFFClose(tif);
    }
    catch (...){
        return false;
    }
    return true;
}


bool get_matrix_element_type(VALUE data_matrix, bool &is_float){
    if (!RB_TYPE_P(data_matrix, T_ARRAY) || RARRAY_LEN(data_matrix) < 1)
        return false;

    VALUE first_row = RARRAY_PTR(data_matrix)[0];

    if (!RB_TYPE_P(first_row, T_ARRAY) || RARRAY_LEN(first_row) < 1)
        return false;

    VALUE first_item = RARRAY_PTR(first_row)[0];

    if (!RB_TYPE_P(first_item, T_FIXNUM) && !RB_TYPE_P(first_item, T_FLOAT))
        return false;

    is_float = RB_TYPE_P(first_item, T_FLOAT);

    return true;
}

template <class F, class T>
vector<vector<T> > matrix_type_cast(vector<vector<F> > & from){
    vector<vector<T> > result;
    result.resize(from.size());

    for (int i = 0; i < from.size(); i++)
        result[i].assign(from[i].begin(), from[i].end());

    return result;
};

VALUE TiffFile = Qnil;

VALUE method_tifffile_converter_to_tiff(VALUE self, VALUE filename, VALUE data_matrix, VALUE sample_size, VALUE sample_unsigned, VALUE description, VALUE software){
    data_matrix = rb_check_array_type(data_matrix);


    Check_Type(filename,        T_STRING);
    Check_Type(data_matrix,     T_ARRAY);
    Check_Type(sample_size,     T_FIXNUM);
    Check_Type(description,     T_STRING);
    Check_Type(software,        T_STRING);

    uint8 size = std::min<uint8>(NUM2USHORT(sample_size), 8);

    bool unsigned_flag = RTEST(sample_unsigned);

    bool is_float_matrix;
    if (!get_matrix_element_type(data_matrix, is_float_matrix))
        rb_raise(rb_eRuntimeError, "Invalid matrix items type");

    if (size != 1 && size != 2 && size != 4 && size != 8)
        rb_raise(rb_eRuntimeError, "Sample size should be in range [1, 2, 4, 8]");

    if (is_float_matrix && (size % 4) != 0)
        rb_raise(rb_eRuntimeError, "Sample size should be 4 or 8 on non-fixnum matrix");

    vector<vector<double> > data_d;
    vector<double>  temp_d;
    vector<vector<int64> >   data_i;
    vector<int64>  temp_i;

    VALUE row, item;

    if (is_float_matrix)
        data_d.resize(RARRAY_LEN(data_matrix));
    else
        data_i.resize(RARRAY_LEN(data_matrix));


    for (long i = 0; i < RARRAY_LEN(data_matrix); i++){
        row = RARRAY_PTR(data_matrix)[i];

        if (!RB_TYPE_P(row, T_ARRAY))
            rb_raise(rb_eRuntimeError, "Matrix row %ld is not an array", i);

        // Clear row buffer
        if (is_float_matrix){
            temp_d.clear();
            temp_d.resize(RARRAY_LEN(row));
        } else {
            temp_i.clear();
            temp_i.resize(RARRAY_LEN(row));
        }


        // Read row
        for (long j = 0; j < RARRAY_LEN(row); j++){
            item = RARRAY_PTR(row)[j];
            Check_Type(item, is_float_matrix ? T_FLOAT : T_FIXNUM);
            if (is_float_matrix)
                temp_d[j] = RFLOAT_VALUE(item);
            else
                temp_i[j] = FIX2LONG(item);
        }

        if (is_float_matrix)
            data_d[i] = temp_d;
        else
            data_i[i] = temp_i;
    }


    bool result = false;
    if (is_float_matrix){
        if (size == 8)
            result = write_tiff_file<double>(StringValuePtr(filename), data_d, 1, 64, StringValuePtr(description), StringValuePtr(software));
        else if (size == 4){
            vector<vector<float> > cast_buffer = matrix_type_cast<double, float>(data_d);
            result = write_tiff_file<float>(StringValuePtr(filename), cast_buffer, 1, 32, StringValuePtr(description), StringValuePtr(software));
        }
        else
            rb_raise(rb_eRuntimeError, "unrecognized sample size for float matrix : %d", size);
    }
    else {
        if (unsigned_flag){
            if (size == 8){
                vector<vector<uint64> > cast_buffer = matrix_type_cast<int64, uint64>(data_i);
                result = write_tiff_file<uint64>(StringValuePtr(filename), cast_buffer, 1, 64, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 4){
                vector<vector<uint32> > cast_buffer = matrix_type_cast<int64, uint32>(data_i);
                result = write_tiff_file<uint32>(StringValuePtr(filename), cast_buffer, 1, 32, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 2){
                vector<vector<uint16> > cast_buffer = matrix_type_cast<int64, uint16>(data_i);
                result = write_tiff_file<uint16>(StringValuePtr(filename), cast_buffer, 1, 16, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 1){
                vector<vector<uint8> > cast_buffer = matrix_type_cast<int64, uint8>(data_i);
                result = write_tiff_file<uint8>(StringValuePtr(filename), cast_buffer, 1, 8, StringValuePtr(description), StringValuePtr(software));
            }
            else
                rb_raise(rb_eRuntimeError, "unrecognized sample size for unsigned numeric matrix : %d", size);
        }
        else {
            if (size == 8){
                result = write_tiff_file<int64>(StringValuePtr(filename), data_i, 1, 64, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 4){
                vector<vector<int32> > cast_buffer = matrix_type_cast<int64, int32>(data_i);
                result = write_tiff_file<int32>(StringValuePtr(filename), cast_buffer, 1, 32, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 2){
                vector<vector<int16> > cast_buffer = matrix_type_cast<int64, int16>(data_i);
                result = write_tiff_file<int16>(StringValuePtr(filename), cast_buffer, 1, 16, StringValuePtr(description), StringValuePtr(software));
            }
            else if (size == 1){
                vector<vector<int8> > cast_buffer = matrix_type_cast<int64, int8>(data_i);
                result = write_tiff_file<int8>(StringValuePtr(filename), cast_buffer, 1, 8, StringValuePtr(description), StringValuePtr(software));
            }
            else
                rb_raise(rb_eRuntimeError, "unrecognized sample size for signed numeric matrix : %d", size);
        }
    }

    return result ? Qtrue : Qfalse;
}

extern "C" void Init_tifffile(){
    TiffFile = rb_define_module("TiffFile");
    rb_define_singleton_method(TiffFile, "to_tiff", (VALUE(*)(ANYARGS))method_tifffile_converter_to_tiff, 6);
}
