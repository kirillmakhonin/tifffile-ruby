require 'tifffile/version'
require "tifffile/tifffile"

module TiffFile

  include TiffFileVersion

  # Will write a TIFF file +destination+, that contains +values+ matrix (in one channel), each sample has +sample_size+ in bytes (1, 2, 4 or 8), +sample_unsigned+.
  # Also, image have +description+ and reference to +software+
  def self.tiff2file(destination, values, sample_size=8, sample_unsigned=false, description=nil, software=nil)
    self.to_tiff(destination.to_s, values, sample_size.to_i, sample_unsigned == true, description.to_s, software.to_s)
  end


  # Binary get TIFF file that contains +values+ matrix (in one channel), each sample has +sample_size+ in bytes (1, 2, 4 or 8), +sample_unsigned+.
  # Also, image have +description+ and reference to +software+
  def self.tiff2binary(values, sample_size=8, description=nil, software=nil)
    file = Tempfile.new('tiff2binary')
    file.close

    tiff2file(file.path, values, sample_size, description, software)

    data = File.read(file)
    file.unlink
    data
  end

end
