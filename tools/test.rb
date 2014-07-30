
class String
  def is_broken
    puts self + " FAILED!"
    exit
  end
end

# Test different password types.
CHARSETS = [
  { long: '--hex', short: '-x', regexp: /\A[0-9A-F]{64}\n\z/ },
  { long: '--alpha', short: '-n', regexp: /\A[0-9a-zA-Z]{64}\n\z/ },
  # (The clever regexp is from http://www.catonmat.net/blog/my-favorite-regex/)
  { long: '--ascii', short: '-a', regexp: /\A[ -~]{64}\n\z/ },
  { long: '--digit', short: '-d', regexp: /\A[0-9]{64}\n\z/ },
  { long: '--lower', short: '-l', regexp: /\A[a-z]{64}\n\z/ },
  { long: '--words', short: '-w', regexp: /\A(([a-z]+)\.){9}[a-z]+\.*\n\z/ },
]

puts "This will take a few minutes..."

CHARSETS.each do |charset|
  output = `./passgen #{charset[:long]} 2>&1`
  "#{charset[:long]} Exit Status".is_broken unless $?.exitstatus == 0
  "#{charset[:long]} Output".is_broken unless charset[:regexp] =~ output

  output = `./passgen #{charset[:short]} 2>&1`
  "#{charset[:long]} Exit Status".is_broken unless $?.exitstatus == 0
  "#{charset[:long]} Output".is_broken unless charset[:regexp] =~ output
end

# Test what happens when /dev/urandom is missing.
output = `fakechroot sh -c "chroot ./ /passgen -x 2>&1"`
"URANDOM Exit Status".is_broken unless $?.exitstatus == 1
"URANDOM Output".is_broken unless output === "ERROR: Runtime self-tests failed. SOMETHING IS WRONG\n"

output = `fakechroot sh -c "chroot ./ /passgen -x -z 2>&1"`
"URANDOM Exit Status".is_broken unless $?.exitstatus == 1
"URANDOM Output".is_broken unless output === "Error getting random data or allocating memory.\n"

# Test multiple ASCII password output
output = `./passgen -a -p 213 2>&1`
"Multiple Exit Status".is_broken unless $?.exitstatus == 0
"Multiple Output".is_broken unless /\A([ -~]{64}\n){213}\z/ =~ output

# Test multiple word password output (it uses different logic)
output = `./passgen -w -p 213 2>&1`
"Word Multiple Exit Status".is_broken unless $?.exitstatus == 0
"Word Multiple Output".is_broken unless /\A((([a-z]+)\.){9}[a-z]+\.*\n){213}\z/ =~ output

# Test negative password count.
output = `./passgen -a -p -2 2>&1`
"Multiple (Negative) Exit Status".is_broken unless $?.exitstatus == 1

# Make sure the first and last word in the wordlist can appear in the output.
# Note: This may false-negative, but the probability of that is extremely low.
words = File.readlines("libs/wordlist.txt").map { |w| w.strip }
output = `./passgen -w -p #{words.count} 2>&1`
"Random Words".is_broken unless output.include?("."+words.first+".") and output.include?("."+words.last+".")

puts "ALL TESTS PASS!"
