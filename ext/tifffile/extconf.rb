require "mkmf"

#LIBDIR      = Config::CONFIG['libdir']
#INCLUDEDIR  = Config::CONFIG['includedir']

HEADER_DIRS = [
    # First search /opt/local for macports
    '/opt/local/include',

    # Then search /usr/local for people that installed from source
    '/usr/local/include',

    # Check the ruby install locations
    #INCLUDEDIR,

    # Finally fall back to /usr
    '/usr/include',
]

LIB_DIRS = [
    # First search /opt/local for macports
    '/opt/local/lib',

    # Then search /usr/local for people that installed from source
    '/usr/local/lib',

    # Check the ruby install locations
    #LIBDIR,

    # Finally fall back to /usr
    '/usr/lib',
]


libtiff_dirs = dir_config('libtiff', '/opt/local/include', '/opt/local/lib')

unless ["", ""] == libtiff_dirs
  HEADER_DIRS.unshift libtiff_dirs.first
  LIB_DIRS.unshift libtiff_dirs[1]
end

unless find_header('tiffio.h', *HEADER_DIRS)
  abort "libtiff is missing.  please install libtiff6-dev"
end

unless find_library('tiff', 'TIFFOpen', *LIB_DIRS)
  abort "libtiff is missing.  please install libtiff-dev"
end



abort "missing malloc()" unless have_func "malloc"
abort "missing free()"   unless have_func "free"


create_makefile "tifffile/tifffile"

