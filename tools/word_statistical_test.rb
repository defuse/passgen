
# Wordlist needs much more samples.
puts "WARNING: This takes about 10-20 hours to run, depending on your system."
PASSWORD_SAMPLES = 10_000_000
SD_THRESHOLD = 3.5

WORDS = File.readlines("libs/wordlist.txt").map { |word| word.strip }

# True if and only if all statistical tests pass.
all_good = true

# Initialize the frequency counter.
counts = Hash.new
WORDS.each do |char|
  counts[char] = 0
end
# Keep track of the actual number of samples.
total = 0

# Run the password generator and count the frequency of each character.
IO.popen( "./passgen --words -p #{PASSWORD_SAMPLES}" ) do |passgen|
  last_percent = 0
  passgen.each do |line|
    line.strip.split('.').each do |char|
      unless counts.has_key? char
        puts "ERROR: Unknown character #{char} in output."
        all_good = false
      end
      counts[char] += 1
      total += 1
      percent = (total.to_f / (PASSWORD_SAMPLES * 10) * 1000).floor
      if percent != last_percent
        puts "#{percent}%..."
        STDOUT.flush
        last_percent = percent
      end
    end
  end
end

# If the actual number of samples isn't what we'd suspect, something is wrong.
if total != PASSWORD_SAMPLES * 10
  puts "ERROR: Incorrect number of samples."
  all_good = false
end

# A character's frequency is like drawing from a box that contains
# 1 one and WORDS.count-1 zeros. Compute the mean and standard deviation for
# that box:
expected_box = 1.0 / WORDS.count
sd_box = Math.sqrt(1.0/WORDS.count * ( (1 - expected_box)**2 + (WORDS.count-1)*(-expected_box)**2 ))

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
WORDS.each do |char|
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

if all_good
  puts "ALL TESTS PASS."
else
  puts "FAILURES!"
end
