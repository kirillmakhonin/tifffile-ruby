require 'tifffile/version'
require "tifffile/tifffile"

module TiffFile

  include TiffFileVersion

  # @sample_size size in bytes. Supported [1, 2, 4, 8]
  def self.tiff2file(destination, values, sample_size=16, sample_unsigned=false, description=nil, software=nil)
    self.to_tiff(destination.to_s, values, sample_size.to_i, sample_unsigned == true, description.to_s, software.to_s)
  end

  def self.tiff2binary(values, sample_size=16, description=nil, software=nil)

  end

end
