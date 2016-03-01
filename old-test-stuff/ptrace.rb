require 'Ptrace'

# launch process 
cmd = './passgen -a'

tgt = Ptrace::Target.launch cmd
loop do
   STDERR.print tgt.regs.read['rip'].to_s + "\n"
   tgt.step
end
continue process
tgt.cont
