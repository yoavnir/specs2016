class Specs < Formula
  desc "Command-line utility for parsing and re-arranging text input"
  homepage "https://github.com/yoavnir/specs2016"
  url "https://github.com/yoavnir/specs2016/archive/refs/tags/v0.9.6.tar.gz"
  # sha256 "UPDATE_WITH_ACTUAL_SHA256_AFTER_RELEASE"
  license "MIT"

  depends_on xcode: :build

  def install
    cd "specs/src" do
      system "python3", "setup.py", "-c", "CLANG", "--python", "no"
      system "make", "some"
    end
    bin.install "specs/exe/specs"
    bin.install "specs/exe/specs-autocomplete"
    man1.install "manpage" => "specs.1"
  end

  test do
    assert_match "specs", shell_output("#{bin}/specs '@version'")
  end
end
