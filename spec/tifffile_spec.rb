require 'tifffile'

describe "TiffFile" do
  it "building tiff file" do
    TiffFile.tiff2file 'a.tiff', [[10, 20], [20, 30]]
  end
end