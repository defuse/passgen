

locations = [
  [0x0000000000400fd4, "<main>:"],
  [0x0000000000401350, "<showHelp>:"],
  [0x00000000004013dd, "<getPassword>:"],
  [0x0000000000401594, "<showRandomWords>:"],
  [0x000000000040182d, "<getLeastCoveringMask>:"],
  [0x000000000040185e, "<getRandom>:"],
  [0x00000000004018e7, "<getRandomUnsignedLong>:"],
  [0x0000000000401906, "<runtimeTests>:"],
  [0x0000000000401f80, "<words>:"],
  [0xFFFFFFFFFFFFFFFF, "END"],
]


f = File.open("addrs.txt")

loop do
  addr = f.readline().to_i
  0.upto(locations.length - 2) do |loc|
    if locations[loc][0] < addr and addr < locations[loc+1][0]
      # HACK
      break if loc == locations.length - 2
      puts " " * (addr - locations[loc][0]) + "X"
      break
    end
  end
end
