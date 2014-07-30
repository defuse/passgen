
if ARGV[0] == "fast"
  puts "WARNING: Fast mode will miss smaller biases."
  PASSWORD_SAMPLES = 100_000
else
  puts "WARNING: This might take a few hours to run..."
  PASSWORD_SAMPLES = 1_000_000 
end
# I tested this script by inserting a bias with rand() % 10000 == 0, and it
# caught it with 1_000_000 password samples. So this many samples can catch
# a bias even up to 1 part in 10,000.

# 1 in 2149 chance of false positive
SD_THRESHOLD = 3.5

# Character Sets
HEX = "0123456789ABCDEF".split('')
ALPHANUMERIC = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789".split('')
ASCII = '!"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~'.split('')
DIGIT = "0123456789".split('')
LOWER = "abcdefghijklmnopqrstuvwxyz".split('')

charsets = {
  "hex" => HEX,
  "alpha" => ALPHANUMERIC,
  "ascii" => ASCII,
  "digit" => DIGIT,
  "lower" => LOWER
}

# True if and only if all statistical tests pass.
all_good = true

charsets.each do |charset_name, charset|
  puts "Testing: #{charset_name}"

  # Initialize the frequency counter.
  counts = Hash.new
  charset.each do |char|
    counts[char] = 0
  end
  # Keep track of the actual number of samples.
  total = 0

  # Run the password generator and count the frequency of each character.
  IO.popen( "./passgen --#{charset_name} -p #{PASSWORD_SAMPLES}" ) do |passgen|
    passgen.each do |line|
      line.strip.split('').each do |char|
        unless counts.has_key? char
          puts "ERROR: Unknown character #{char} in output."
          all_good = false
        end
        counts[char] += 1
        total += 1
      end
    end
  end

  # If the actual number of samples isn't what we'd suspect, something is wrong.
  if total != PASSWORD_SAMPLES * 64
    puts "ERROR: Incorrect number of samples."
    all_good = false
  end

  # A character's frequency is like drawing from a box that contains
  # 1 one and charset.count-1 zeros. Compute the mean and standard deviation for
  # that box:
  expected_box = 1.0 / charset.count
  sd_box = Math.sqrt(1.0/charset.count * ( (1 - expected_box)**2 + (charset.count-1)*(-expected_box)**2 ))

  # Now, compute the expected value and standard error of the sum.
  expected = expected_box * total
  sd_sum = sd_box * Math.sqrt(total)

  # Start the results table.
  puts "\n    TOTAL SAMPLES: #{total}"
  puts  "    STANDARD DEVIATION THRESHOLD: #{SD_THRESHOLD}"
  puts  "    +--------+------------+------------------------+-----------------+"
  print "    | %6s | %-10s | %-22s | %-15s |\n" % ["char", "total", "sd", "status"]
  puts  "    +--------+------------+------------------------+-----------------+"

  # Print a row for each character.
  charset.each do |char|
    # Count how many standard deviations the actual sum is away from the
    # expected.
    difference_sds = (counts[char] - expected).abs / sd_sum
    # If it's within only a few standard deviations, everything is good. But if
    # it's outside that, it's good evidence that there's some bias.
    if difference_sds > SD_THRESHOLD
      status = "*****FAIL!*****"
      all_good = false
    else
      status = "PASS."
    end
    print "    | %6s | %-10s | %-22s | %-15s |\n" % [char, counts[char].to_s, difference_sds.to_s, status]
  end
  print  "    +--------+------------+------------------------+-----------------+\n\n"

end # charsets.each

if all_good
  puts "ALL TESTS PASS."
  exit(0)
else
  puts "FAILURES!"
  exit(1)
end
