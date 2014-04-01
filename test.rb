
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

output = `./passgen -w -p 213 2>&1`
"Word Multiple Exit Status".is_broken unless $?.exitstatus == 0
"Word Multiple Output".is_broken unless /\A((([a-z]+)\.){9}[a-z]+\n){213}\z/ =~ output

# Make sure the first and last word in the wordlist can appear in the output.
# Note: This may false-negative, but the probability of that is extremely low.

puts "Sorry, this will take a few minutes..."
words = File.readlines("wordlist.txt").map { |w| w.strip }
output = `./passgen -w -p #{words.count} 2>&1`
"Random Words".is_broken unless output.include?("."+words.first+".") and output.include?("."+words.last+".")

puts "ALL TESTS PASS!"
