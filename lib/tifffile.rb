require 'tifffile/version'
require "tifffile/tifffile"

class TiffFile

  include TiffFileVersion

  def self.tiff2file(destination, values, photometric=nil, planarconfig=nil, description=nil, software=nil, byteorder=nil, bigtiff=nil, compress=nil, extratags=nil)
    data2tiff(destination.to_s, values, photometric, planarconfig, description, software, byteorder, bigtiff, compress, extratags)
  end

  def self.tiff2binary(values, photometric=nil, planarconfig=nil, description=nil, software=nil, byteorder=nil, bigtiff=nil, compress=nil, extratags=nil)

  end

end
