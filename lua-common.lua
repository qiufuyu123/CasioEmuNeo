local hwid = emu:model().hardware_id
local stack_end = hwid == 3 and 0x8e00 or 0xf000
local screen_nrow = hwid == 3 and 32 or 64
local screen_ncol = hwid == 3 and 12 or 24 -- bytes = 8 pixels
local screen_row_width = hwid == 3 and 16 or 32 -- > screen_ncol

local break_targets = {}

local posttickfns = {}

function addposttick(fn)
	if type(fn) ~= 'function' then
		print('Argument is not a function')
		return
	end
	if not next(posttickfns) then
		emu:post_tick(function()
			for i, fn in pairs(posttickfns) do
				fn()
			end
		end)
	end
	table.insert(posttickfns, fn)
end

function rmposttick(fn)
	if fn == nil then
		if not next(posttickfns) then
			print('There is no posttick handler')
		end
		posttickfns[#posttickfns] = nil
	else
		for i,f in pairs(posttickfns) do
			if f==fn then
				table.remove(posttickfns,i)
				return
			end
		end
		print('Posttick handler not found')
	end

	if not next(posttickfns) then
		emu:post_tick(nil)
	end
end

local function get_real_pc()
	return (cpu.csr << 16) | cpu.pc & ~1
end

function break_at(addr, commands)
	if not addr then
		addr = get_real_pc()
	end
	if commands then
		if type(commands) ~= 'function' then
			printf('Invalid secomd argument to break_at: %s', commands)
			return
		end
	else
		commands = function() end
	end

    if not next(break_targets) then
        -- if break_targets is initially empty and later non-empty
		addposttick(break_posttick)
	end

	break_targets[addr] = commands
end

function unbreak_at(addr)
	if not addr then
		addr = get_real_pc()
	end
	break_targets[addr] = nil

    if not next(break_targets) then
        rmposttick(break_posttick)
    end
end

function cont()
	emu:set_paused(false)
end

function break_posttick()
	local real_pc = get_real_pc()
	local commands = break_targets[real_pc]
	if commands then
		emu:set_paused(true)
		commands()
	end
end

function printf(...)
	print(string.format(...))
end

function ins()
	printf("%02X %02X %02X %02X | %01X:%04X | %02X %01X:%04X", cpu.r0, cpu.r1, cpu.r2, cpu.r3, cpu.csr,  cpu.pc, cpu.psw, cpu.lcsr, cpu.lr)
	printf("%02X %02X %02X %02X | S %04X | %02X %01X:%04X", cpu.r4, cpu.r5, cpu.r6, cpu.r7, cpu.sp, cpu.epsw1, cpu.ecsr1, cpu.elr1)
	printf("%02X %02X %02X %02X | A %04X | %02X %01X:%04X", cpu.r8, cpu.r9, cpu.r10, cpu.r11, cpu.ea, cpu.epsw2, cpu.ecsr2, cpu.elr2)
	printf("%02X %02X %02X %02X | ELVL %01X | %02X %01X:%04X", cpu.r12, cpu.r13, cpu.r14, cpu.r15, cpu.psw & 3, cpu.epsw3, cpu.ecsr3, cpu.elr3)
end

function help()
	print([[
The supported functions are:

printf()        Print with format.
ins             Log all register values to the screen.
break_at        Set breakpoint.
                If input not specified, break at current address.
                Second argument (optional) is a function that is executed whenever
                this breakpoint hits.
unbreak_at      Delete breakpoint.
                If input not specified, delete breakpoint at current address.
                Have no effect if there is no breakpoint at specified position.
cont()          Continue program execution.
inject          Inject 100 bytes to the input field.
pst()           Print 48 bytes of the stack before and after SP.

emu:set_paused  Set emulator state.
emu:tick()      Execute one command.
emu:shutdown()  Shutdown the emulator.

cpu.xxx         Get register value.
cpu.bt          Current stack trace.

code            Access code. (By words, only use even address,
                otherwise program will panic)
data            Access data. (By bytes)
data:watch      Set write watchpoint.
data:rwatch     Set read watchpoint.
help()          Print this help message.

addposttick     Add a function as post-tick handler. (wrapper over emu:post_tick)
rmposttick      Remove a post-tick handler. If called without argument,
                delete the most-recently added handler.

er(x)           Value of register ERx.

GDB-style functions:
(the long functions are intended to be used in code, the short functions are intended
to be used interactively so they prints debug info/etc.)

p(...)          Shorthand for `print`.
pn              Print calculator number at address
ps/gets         Print/get calculator string (0-terminated) at address
pi/geti         Print/get 2-byte unsigned integer at address
h()             help()
n()/nexti()     Go to next instruction.
c()             cont
s()             emu:tick()
tr(filename)    Start tracing (record all executed instructions)
trs()           Stop tracing.
q()             emu:shutdown()
b               break/pause (breakpoint set with b should not be deleted with unbreak_at)
del             Delete breakpoint or watchpoint
bt              print(cpu.bt)
rwa             Set read watchpoint at location
                   (watchpoints set with rwa should not be deleted with data:rwatch)
wa              Set watchpoint at location
                   (watchpoints set with wa should not be deleted with data:watch)
u0/until0       Run until address is hit and sp is <= original sp.
                   Not as fully-functional as gdb's `until` command, so `u0`.
ppc()           Print current PC address.
calll           Call log. (addr, before, after)
nrop            Next "ROP instruction".
]])
end

local function get_real_lr()
	return (cpu.lcsr << 16) | cpu.lr
end

local function to_function(x)
	-- helper function to make entering commands easier
	if type(x) == 'function' or x == nil then
		return x
	end
	x = loadstring(x)
	return x
end

function readdis(filename)
	-- Read a disassembly file.
	local handle = io.open(filename, "r")
	if not handle then
		printf("Failed to open \"%s\"", filename)
		return
	end
	local name_content = handle:read("*a")
	handle:close()

	local dis_lines, line_by_addr, label_by_addr = {}, {}, {}
	local last_global_label, last_label = '', ''
	local last_label_offset = 0 -- distance to last label
	local last_addr = nil -- this is nil when the last non-comment line is a label
	for line in name_content:gmatch("[^\n]+") do
		dis_lines[#dis_lines+1] = line

		if line:sub(#line) == ':' then
			local label = line:sub(1, #line-1)
			if label:sub(1, 1) == '.' then
				label = last_global_label .. label
			else
				last_global_label = label
			end
			last_addr = nil
			last_label = label
			last_label_offset = 0
		end

		local addr = line:match('; ([0-9A-F]+) | ')
		if addr then
			addr = tonumber(addr, 16)
			line_by_addr[addr] = #dis_lines
			if last_addr then
				last_label_offset = last_label_offset + addr - last_addr
			end
			last_addr = addr

			local label = last_label
			if last_label_offset > 0 then
				if #label > 0 then
					label = label .. '+'
				end
				label = label .. ('%X'):format(last_label_offset)
			end
			label_by_addr[addr] = label
		end
	end

	return dis_lines, line_by_addr, label_by_addr
end

function until0(addr)
	local old_sp = cpu.sp
	while true do
		emu:tick()
		if get_real_pc() == addr and cpu.sp >= old_sp then
			break
		end
	end
end

function getn_str(x) -- Calculator 10-byte decimal fp number to string.
	-- `x` should be a list of 10 integers
	-- Return nil for invalid number (numbers with hexadecimal digits in the mantissa part are considered valid)
	-- If there's any hexadecimal characters in the number they're returned in uppercase.

	if not ({[0]=true,[1]=true,[5]=true,[6]=true})[x[10]] then
		return nil
	end
	if x[1]>=16 then
		return nil
	end

	for i=1,9 do x[i]=("%02X"):format(x[i]) end
	local a = x[1]:sub(2,2) .. '.' .. table.concat(x):sub(3,16)
	local exp = tonumber(x[9])
	if exp == nil then return nil end

	if x[10]>4 then a = "-"..a end
	if x[10]==0 or x[10]==5 then exp = exp - 100 end
	a=a.."e"..exp

	if a=="0.00000000000000e-100" then return "0" end

	if a:sub(1,1)~="0" then -- prettify a
		b=tonumber(a)
		if b~=nil then return ("%.15g"):format(b) end
	end
	return a
end

function getn_data(x) -- Calculator 10-byte decimal fp number to float.
	-- `x` should be a list of 10 integers
	-- Return nil for invalid number.
	return tonumber(getn_str(x))
end

function getn(adr)
	local x = {}
	for i = 0,9 do
		x[i+1] = data[adr+i]
	end
	local a = getn_str(x)
	if a then return a else return '<?>' end
end

function er(x)
	assert (0 <= x and x < 16 and x % 2 == 0)
	return cpu['r' .. (x+1)] << 8 | cpu['r' .. x]
end

function ppc()
	local pc = get_real_pc()
	if label_by_addr and label_by_addr[pc] then
		pc = label_by_addr[pc]
	else
		pc = ('%06X'):format(pc)
	end
	printf(' ** Reached address %s **', pc)
end

function nexti()
	-- Use a simple heuristic that doesn't require reading the opcode
	-- or cooperation from the CPU. However this is not correct.
	-- (in the future, this may be implemented with CPU help)
	local old_pc = get_real_pc()
	local old_sp = cpu.sp
	emu:tick()
	local new_lr = get_real_lr()
	local new_pc = get_real_pc()
	local new_sp = cpu.sp
	if not ((new_lr == old_pc + 2 or new_lr == old_pc + 4) and old_sp == new_sp and
			(new_pc ~= old_pc + 2 and new_pc ~= old_pc + 4)) then
		return
	end

	while true do
		emu:tick()
		if get_real_pc() == new_lr and cpu.sp == old_sp then
			-- done
			break
		end
	end
end

function gets(adr, maxlen)
	maxlen = maxlen or 20
	str = ''
	while true do
		byte = data[adr]
		if byte == 0 then
			return str
		end
		str = str .. string.char(byte)
		if #str > maxlen then
			return str
		end
		adr = adr + 1
	end
end

function geti(adr)
	return data[adr+1] << 8 | data[adr]
end

function s() emu:tick() ppc() end

local hexadigit = '[0-9A-F]'
-- hexadecimal digits, 4 <= len <= 6
local addr_pat = hexadigit:rep(4) .. (hexadigit..'?'):rep(2)
function bt()
	local result = cpu.bt
	if label_by_addr then
		-- monkey-patch the result...
		result = result:gsub(addr_pat, function(addr)
			return label_by_addr[tonumber(addr, 16)] or addr
		end)
	end
	print(result)
end

function l(addr)
	-- Print disassembly around the address addr. If not passed, use PC.
	if not line_by_addr or not dis_lines then
		print('disassembly not available')
		return
	end
	if not addr then
		addr = get_real_pc()
	end
	local cur_line = line_by_addr[addr]
	if not cur_line then
		print('address has no corresponding line')
		return
	end

	if not l_hl then
		print([[
Please configure l_hl for current line highlighting.  Example:

	l_hl={hl_start='>>>',hl_stop='<<<',no_start='',no_stop=''

with 'hl': highlighted, 'no': not highlighted.]])
		return
	end
	local min_line = math.max(1, cur_line - 5)
	local max_line = math.min(#dis_lines, cur_line + 4)
	for line = min_line, max_line do
		print((line == cur_line and l_hl.hl_start or l_hl.no_start)
			.. dis_lines[line]
			.. (line == cur_line and l_hl.hl_stop or l_hl.no_stop))
	end
end

function q() emu:shutdown() end
function c() cont() end
h = help
function u0(addr) until0(addr) ppc() end

function calll(addr, before, after)
	-- Call before every time function addr is called and call after every time
	-- function addr returns.
	before = to_function(before)
	after = to_function(after)
	b(addr, function()
		before()
		until0(get_real_lr())
		after()
		cont()
	end)
end

function fnl(addr, narg, name)
	if narg ~= 1 and narg ~= 2 then
		print('narg must be 1 or 2')
		return
	end
	calll(addr, function()
		local old_er0 = er(0)
		printf(' ** fnl [%s] / %06X ** ', name, addr)
		pn(old_er0)
		if narg == 2 then
			pn(er(2))
		end
		prs() ins()
	end,function()
		print('-----> (old er0) ')
		pn(old_er0)
		prs() ins()
		print('==============================')
	end)
end

function n()
	nexti()
	ppc()
end

local numbered_breakpoints = {}
local last_number = 0

function b(addr, commands)
	-- Pause the program if no argument is given.
	if not addr then
		emu:set_paused(true)
		return
	end

	if not commands then
		commands = function()
			printf('** breakpoint reached at %06X **', get_real_pc())
		end
	end

	if break_targets[addr] then
		print('Warning: Override an existing breakpoint')
	end
	last_number = last_number + 1
	numbered_breakpoints[last_number] = {addr=addr, type_='break'}
	printf('Breakpoint %d set at %06X', last_number, addr)
	break_at(addr, to_function(commands))
end

function ib() -- info breakpoints
	for num, info in pairs(numbered_breakpoints) do
		printf('%6s %2d at %06X', info.type_, num, info.addr)
	end
	if not next(numbered_breakpoints) then
		p('No breakpoints or watchpoints.')
	end
end

function wa(addr, commands)
	if not commands then
		commands = function()
			printf('** watchpoint on %06X reached at %06X **', addr, get_real_pc())
			emu:set_paused(true)
		end
	end

	last_number = last_number + 1
	numbered_breakpoints[last_number] = {addr=addr, type_='watch'}
	printf('Watchpoint %d set at %06X', last_number, addr)
	data:watch(addr, to_function(commands))
end

function rwa(addr, commands)
	if not commands then
		commands = function()
			printf('** read watchpoint on %06X reached at %06X **', addr, get_real_pc())
			emu:set_paused(true)
		end
	end

	last_number = last_number + 1
	numbered_breakpoints[last_number] = {addr=addr, type_='rwatch'}
	printf('Read watchpoint %d set at %06X', last_number, addr)
	data:rwatch(addr, commands)
end

function del(num)
	if numbered_breakpoints[num] then
		if numbered_breakpoints[num].type_ == 'break' then
			unbreak_at(numbered_breakpoints[num].addr)
		elseif numbered_breakpoints[num].type_ == 'watch' then
			data:watch(numbered_breakpoints[num].addr, nil)
		elseif numbered_breakpoints[num].type_ == 'rwatch' then
			data:rwatch(numbered_breakpoints[num].addr, nil)
		else
			print('Unknown type!?')
		end

		numbered_breakpoints[num] = nil
		if not next(numbered_breakpoints) then
			last_number = 0
		end
	else
		print('Breakpoint/watchpoint does not exist or deleted')
	end
end

p = print

function pn(adr)
	print(getn(adr))
end

function getrn_str(adr) -- Similar to getn_str, but with register order
	local x = {}
	for j=1,8 do
		x[j] = data[adr+10-j]
	end
	x[9] = data[adr]
	x[10] = data[adr+1]
	return getn_str(x)
end

function getrn_data(adr) -- Similar to getn_data, but with register order
	return tonumber(getrn_str(x))
end

function prs() -- Print calculator number registers (specific to ES+)
	local x = {0x8000, 0x8010, 0x8020, 0x8030, 0x803C, 0x8046, 0x8050}
	for i,v in ipairs(x) do x[i]=getrn_str(v) end

	printf(' R0: %20s | R1: %20s | R2: %20s',x[1],x[2],x[3])
	printf(' R3: %20s | R4: %20s | R5: %20s',x[4],x[5],x[6])
	printf(' R6: %20s',x[7])
end

function ps(adr) -- Calculator string
	if not adr then
		error('Parameter is invalid. Perhaps you intended to call pst (print stack)?')
	end
	ans = gets(adr, 20)
	if #ans > 20 then
		print("'" .. ans .. '~~')
	else
		print("'" .. ans .. "'")
	end
end

function pi(adr)
	print(geti(adr))
end

function pst(radius)
	radius = radius or 48

	sp = cpu.sp
	w = io.write
	linecnt = 0
	for i = sp-radius, sp+radius-1 do
		if i >= stack_end then
			break
		end

		w(  ('%s%02x'):format(i==sp and '*' or ' ', data[i])  )
		linecnt = linecnt+1
		if linecnt==16 then
			w('\n')
			linecnt = 0
		end
	end
	p()
end

function nrop()
	local old_sp=cpu.sp
	while true do
		emu:tick()
		if code[cpu.csr<<16|cpu.pc&~1]==0xa1ea -- mov sp, er14
			and er(14)<cpu.sp
			or cpu.sp>old_sp then break end
	end
	ppc()
end

function getscr(d)
	-- Convert a table of 96*32 bytes to string representation.
	local lines={}
	for row=0,screen_nrow-1,2 do
		local line=''
		for col=1,screen_ncol do
			local byte1=d[row*screen_ncol+col]
			local byte2=d[(row+1)*screen_ncol+col]
			for bit=1,4 do
				local x=((byte1&0xc0)>>4)+((byte2&0xc0)>>6)
				-- assume this file is saved in utf-8, each block-drawing character
				-- takes 3 bytes
				line=line..(x<1 and ' ' or ('▗▖▄▝▐▞▟▘▚▌▙▀▜▛█'):sub(x*3-2,x*3))
				byte1=byte1<<2
				byte2=byte2<<2
			end
		end
		table.insert(lines,line)
	end
	return table.concat(lines,'\n')
end

function pscr()
	local d={}
	for row=0,screen_nrow-1 do
		for col=0,screen_ncol-1 do
			table.insert(d,data[0xf800+screen_row_width*row+col])
		end
	end
	print(getscr(d))
end

function pbuf(x)
	local base=hwid==3 and 0x87d0 or (x==2 and 0xe3d4 or 0xddd4)
	local d={}
	for i=base,base+screen_nrow*screen_ncol-1 do
		table.insert(d,data[i])
	end
	print(getscr(d))
end

local trace_handle, trace_last_pc = nil, nil

local function trace_posttick()
	local pc = get_real_pc()
	if pc ~= trace_last_pc then
		local indent = ('  '):rep(#cpu.bt:gsub('[^\n]',''))
		if line_by_addr[pc] then
			trace_handle:write(('%s%s %s\n'):format(indent,
					dis_lines[line_by_addr[pc]]:sub(2,39),
					label_by_addr[pc]
			))
		else
			trace_handle:write(('%s%s; %05X\n'):format(indent,
				(' '):rep(31), pc
			))
		end
		trace_last_pc = pc
	end
end

function tr(filename)
	if trace_handle ~= nil then
		print('Trace already turned on')
		return
	end
	trace_handle = io.open(filename or 'log', 'w')
	trace_last_pc = nil
	addposttick(trace_posttick)
end

function trs()
	if not trace_handle then
		print('Trace is not turned on')
		return
	end
	rmposttick(trace_posttick)
	trace_handle:close()
	trace_handle = nil
end
