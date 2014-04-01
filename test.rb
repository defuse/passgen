
class String
  def is_broken
    puts self + " FAILED!"
    exit
  end
end

# Test different password types.

output = `./passgen --hex 2>&1`
"HEX Exit Status".is_broken unless $?.exitstatus == 0
"HEX Output".is_broken unless /\A[0-9A-F]{64}\n\z/ =~ output

output = `./passgen -x 2>&1`
"HEX Exit Status".is_broken unless $?.exitstatus == 0
"HEX Output".is_broken unless /\A[0-9A-F]{64}\n\z/ =~ output

output = `./passgen --alpha 2>&1`
"ALPHANUMERIC Exit Status".is_broken unless $?.exitstatus == 0
"ALPHANUMERIC Output".is_broken unless /\A[0-9a-zA-Z]{64}\n\z/ =~ output

output = `./passgen -n 2>&1`
"ALPHANUMERIC Exit Status".is_broken unless $?.exitstatus == 0
"ALPHANUMERIC Output".is_broken unless /\A[0-9a-zA-Z]{64}\n\z/ =~ output

output = `./passgen --ascii 2>&1`
"ASCII Exit Status".is_broken unless $?.exitstatus == 0
"ASCII Output".is_broken unless /\A[ -~]{64}\n\z/ =~ output

output = `./passgen -a 2>&1`
"ASCII Exit Status".is_broken unless $?.exitstatus == 0
"ASCII Output".is_broken unless /\A[ -~]{64}\n\z/ =~ output

# (The clever regexp is from http://www.catonmat.net/blog/my-favorite-regex/)

output = `./passgen --words 2>&1`
"Word Exit Status".is_broken unless $?.exitstatus == 0
"Word Output".is_broken unless /\A(([a-z]+)\.){9}[a-z]+\n\z/ =~ output

output = `./passgen -w 2>&1`
"Word Exit Status".is_broken unless $?.exitstatus == 0
"Word Output".is_broken unless /\A(([a-z]+)\.){9}[a-z]+\n\z/ =~ output

# Test what happens when /dev/urandom is missing.
output = `fakechroot sh -c "chroot ./ /passgen -x 2>&1"`
"URANDOM Exit Status".is_broken unless $?.exitstatus == 1
"URANDOM Output".is_broken unless output === "ERROR: Runtime self-tests failed. SOMETHING IS WRONG\n"

output = `fakechroot sh -c "chroot ./ /passgen -x -z 2>&1"`
"URANDOM Exit Status".is_broken unless $?.exitstatus == 1
"URANDOM Output".is_broken unless output === "Error getting random data.\n"

# Test multiple password output

output = `./passgen -a -p 213 2>&1`
"Multiple Exit Status".is_broken unless $?.exitstatus == 0
"Multiple Output".is_broken unless /\A([ -~]{64}\n){213}\z/ =~ output

output = `./passgen -w -p 195 2>&1`
"Word Exit Status".is_broken unless $?.exitstatus == 0
"Word Output".is_broken unless /\A((([a-z]+)\.){9}[a-z]+\n){195}\z/ =~ output

puts "ALL TESTS PASS!"
