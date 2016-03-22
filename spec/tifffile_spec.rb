require 'tifffile'

describe "TiffFile" do
  it "building tiff file" do
    p TiffFile.tiff2file 'a.tiff', [[10, 21], [20, 30]], 8, false, "Image description", "SOFTWARE"
  end
end