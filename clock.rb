require 'date'
require 'serialport'
require 'optparse'
require 'rpi_gpio'

RPi::GPIO.set_numbering :bcm
PIN = 23
RPi::GPIO.setup PIN, :as => :input, :pull => :up

$dry_run = false
from_zero = false
do_reset = false
initial_time = ''
storage_count = 0
run_fast = false
do_move_test = false
$verbose = false
do_goto = false
goto_x = 0
goto_y = 0
replay_file = nil

OptionParser.new do |opts|
  opts.banner = "Usage: clock.rb [options] [portnumber]"

  opts.on("-n", "--dry-run", "Dry run") do |n|
    $dry_run = true
  end
  opts.on("-r", "--reset", "Reset") do |n|
    do_reset = true
  end
  opts.on("-t", "--time TIME", "Current time") do |n|
    initial_time = n
  end
  opts.on("-s", "--storage COUNT", "Balls in storage") do |n|
    storage_count = n.to_i
  end
  opts.on("--replay FILE", "Replay move commands from file") do |n|
    replay_file = n
  end
  opts.on("-f", "--fast", "Run fast") do |n|
    run_fast = true
  end
  opts.on("-m", "--movetest", "Run move test") do |n|
    do_move_test = true
  end
  opts.on("-g x, y", "--goto x,y", Array, "Go to specified coordinate") do |c|
    do_goto = true
    goto_x = c[0]
    goto_y = c[1]
  end
  opts.on("-v", "--verbose", "Verbose") do |n|
    $verbose = true
  end
end.parse!

one   = '  .   ..  . .    .    .    .  .....' # 13
two   = ' ... .   .    .  ..  .   .    .....' # 15
three = ' ... .   .    .  ..     ..   . ... ' # 14
four  = '   .   ..  . . .  . .....   .    . ' # 14
five  = '......    ....     .    ..   . ... ' # 17
six   = '  ..  .   .    .... .   ..   . ... ' # 15
seven = '.....    .   .   .   .    .    .   ' # 11
eight = ' ... .   ..   . ... .   ..   . ... ' # 17
nine  = ' ... .   ..   . ....    .   .  ..  ' # 15
zero  = ' ... .   ..   ..   ..   ..   . ... ' # 16
$dots = [ zero, one, two, three, four, five, six, seven, eight, nine ]

ROWS = 7
COLUMNS = 5
Y_OFFSET = 6
Y_ZERO = 0
WIDTH = 5+2

# First storage: (0, Y_ZERO) to (4, Y_ZERO)
# First digit:   (0, Y_ZERO+Y_OFFSET) to (0, Y_ZERO+Y_OFFSET+6)

$abort_count = 0

def fatal(s)
  puts("FATAL ERROR: #{s}")
  path = File.expand_path(File.dirname(__FILE__))
  puts(path)
  File.write('error.txt', s)
  Process.exit()
end
  
def check_abort()
  if RPi::GPIO.low? 23
    puts "pressed"
    $abort_count = $abort_count+1
  end
  if $abort_count > 3
    puts "ABORT"
    Process.exit()
  end
end

def verbose(s)
  if $verbose
    puts s
  end
end

def wait_response(s)
  begin
    line = $sp.gets
  end while !line || line.empty?
  line.strip!
  verbose "Reply: #{line}"
  expected = "OK #{s}"
  if line[0..expected.size-1] != expected
    fatal("Expected 'OK #{s}', got '#{line}'")
  end
end

def debug(s)
  #puts(s)
end

def dump(dots)
  verbose "-----"
  for i in 0..ROWS-1
    verbose dots[ROWS-1-i]
  end
  verbose "-----"
end

def get_matrix(digit)
  a = []
  bits = $dots[digit]
  for i in 0..ROWS-1
    a[i] = bits[i*COLUMNS..(i+1)*COLUMNS-1]
  end
  return a.reverse
end

def move_ball(index,
              from_x, from_y,
              to_x, to_y)
  verbose("-- #{index}: Move ball (#{from_x}, #{from_y}) to (#{to_x}, #{to_y})")
  x1 = index*WIDTH+from_x 
  y1 = from_y + Y_ZERO + Y_OFFSET
  x2 = index*WIDTH+to_x 
  y2 = to_y + Y_ZERO + Y_OFFSET
  if !$dry_run
    s = "M #{x1.to_i()} #{y1.to_i()} #{x2.to_i()} #{y2.to_i()}"
    verbose "> #{s}"
    $sp.flush_input
    $sp.puts s
    wait_response(s)
  end
  check_abort()
end

$storage = []
for i in 0..4
  a = []
  for j in 0..COLUMNS
    a.push(0)
  end
  $storage.push(a)
end

def move_to_storage(index,
                    from_x, from_y)
  verbose("-- #{index}: Move ball (#{from_x}, #{from_y}) to storage")
  x1 = index*WIDTH+from_x 
  y1 = from_y + Y_ZERO + Y_OFFSET
  storage_digit_index = index
  storage_column_index = 0
  while ($storage[storage_digit_index][storage_column_index] > 0) && (storage_column_index < COLUMNS)
    verbose "Storage #{storage_digit_index}, #{storage_column_index} is used"
    storage_column_index = storage_column_index + 1
    if storage_column_index >= COLUMNS
      storage_column_index = 0
      break
    end
  end
  if $storage[storage_digit_index][storage_column_index] > 0
    # No room at current digit. Look at the other digits
    if index == 0
      candidates = [ 1, 2, 3 ]
    elsif index == 1
      candidates = [ 0, 2, 3 ]
    elsif index == 2
      candidates = [ 1, 3, 0 ]
    elsif index == 3
      candidates = [ 2, 1, 0 ]
    end
    for storage_digit_index in candidates
      for storage_column_index in 0..COLUMNS-1
        if ($storage[storage_digit_index][storage_column_index] == 0)
          break
        end
        verbose "Storage #{storage_digit_index}, #{storage_column_index} is used"
      end
      if ($storage[storage_digit_index][storage_column_index] == 0)
        break
      end
    end
    if ($storage[storage_digit_index][storage_column_index] > 0)
      fatal("Not enough storage")
    end
  end
  $storage[storage_digit_index][storage_column_index] = 1
  verbose "Mark storage #{storage_digit_index}, #{storage_column_index} as used"
  x2 = storage_digit_index*WIDTH+storage_column_index
  y2 = Y_ZERO
  puts "TO STORAGE: #{storage_digit_index} #{storage_column_index} => #{x2}, #{y2}"
  check_abort()
  if !$dry_run
    s = "M #{x1.to_i()} #{y1.to_i()} #{x2.to_i()} #{y2.to_i()}"
    verbose "> #{s}"
    $sp.flush_input
    $sp.puts s
    wait_response(s)
  end
end

def fetch_from_storage(index,
                       to_x, to_y)
  verbose("-- #{index}: Move ball from storage to (#{to_x}, #{to_y})")
  # Start looking at current digit
  storage_digit_index = index
  storage_column_index = COLUMNS-1
  while ($storage[storage_digit_index][storage_column_index] == 0) && (storage_column_index > 0)
    verbose "Storage #{storage_digit_index}, #{storage_column_index} is empty"
    storage_column_index = storage_column_index-1
  end
  verbose("-- #{index}: Storage at #{storage_column_index}: #{$storage[storage_digit_index][storage_column_index]}")  
  if ($storage[storage_digit_index][storage_column_index] == 0)
    # No balls found at current digit. Look at the other digits
    verbose("-- #{index}: Look in other bins")
    if index == 0
      candidates = [ 1, 2, 3 ]
    elsif index == 1
      candidates = [ 0, 2, 3 ]
    elsif index == 2
      candidates = [ 1, 3, 0 ]
    elsif index == 3
      candidates = [ 2, 1, 0 ]
    end
    for storage_digit_index in candidates
      storage_column_index = COLUMNS-1
      while ($storage[storage_digit_index][storage_column_index] == 0) && (storage_column_index > 0)
        verbose "Storage #{storage_digit_index}, #{storage_column_index} is empty"
        storage_column_index = storage_column_index-1
      end
      if ($storage[storage_digit_index][storage_column_index] > 0)
        verbose("-- Found in bin #{storage_digit_index} at #{storage_column_index}")
        break
      end
    end
    if ($storage[storage_digit_index][storage_column_index] == 0)
      fatal("Storage is empty")
    end
  end
  $storage[storage_digit_index][storage_column_index] = 0
  x1 = storage_digit_index*WIDTH+storage_column_index
  y1 = Y_ZERO
  x2 = index*WIDTH+to_x
  y2 = to_y + Y_ZERO + Y_OFFSET
  check_abort()
  if !$dry_run
    s = "M #{x1.to_i()} #{y1.to_i()} #{x2.to_i()} #{y2.to_i()}"
    verbose "> #{s}"
    $sp.flush_input
    $sp.puts s
    wait_response(s)
  end
end

def change(index, prev, cur)
  debug "Change #{index} from #{prev} to #{cur}"
  # Find out how many net balls we need to add/remove
  prev_dots = get_matrix(prev)
  cur_dots = get_matrix(cur)
  prev_count = prev_dots.join.count('.')
  cur_count = cur_dots.join.count('.')
  if (cur_count > prev_count)
    puts "Add #{cur_count - prev_count} balls"
  elsif (cur_count < prev_count)
    puts "Remove #{prev_count - cur_count} balls"
  else
    puts "Only move"
  end
  # While possible, move balls within this digit
  moves = 0
  begin
    add_pos = nil
    rem_pos = nil
    moved = false
    for y in 0..ROWS-1
      p = prev_dots[y]
      c = cur_dots[y]
      for x in 0..COLUMNS-1
        if p[x] != c[x]
          debug "At #{x}, #{y}: #{p[x]} -> #{c[x]}"
          if p[x] == ' '
            # ball must be added
            if !add_pos
              add_pos = [x, y]
            end
          else
            # ball must be removed
            if !rem_pos
              rem_pos = [x, y]
            end
          end
        end
        if add_pos && rem_pos
          break
        end
      end
      if add_pos && rem_pos
        break
      end
    end
    debug "Add: #{add_pos} Remove: #{rem_pos}"
    if add_pos && rem_pos
      move_ball(index,
                rem_pos[0], rem_pos[1],
                add_pos[0], add_pos[1])
      prev_dots[add_pos[1]][add_pos[0]] = '.'
      prev_dots[rem_pos[1]][rem_pos[0]] = ' '
      dump(prev_dots)
      moved = true
      moves = moves + 1
    end
  end until !moved
  puts "Moved #{moves} balls"
  # Are we done?
  prev_count = prev_dots.join.count('.')
  if prev_count > cur_count
    # No, we need to move balls to storage
    begin
      rem_pos = nil
      for y in 0..ROWS-1
        p = prev_dots[y]
        c = cur_dots[y]
        for x in 0..COLUMNS-1
          if p[x] != c[x]
            debug "At #{x}, #{y}: #{p[x]} -> #{c[x]}"
            if p[x] == ' '
              abort('Logic error: Too few balls')
            else
              # ball must be removed
              rem_pos = [x, y]
              break
            end
          end
        end
        if rem_pos
          break
        end
      end
      if rem_pos
        move_to_storage(index, rem_pos[0], rem_pos[1])
        prev_dots[rem_pos[1]][rem_pos[0]] = ' '
        dump(prev_dots)
      end
    end until !rem_pos
  elsif prev_count < cur_count
    # No, we need to fetch balls from storage
    begin
      add_pos = nil
      for y in 0..ROWS-1
        p = prev_dots[y]
        c = cur_dots[y]
        for x in 0..COLUMNS-1
          if p[x] != c[x]
            debug "At #{x}, #{y}: #{p[x]} -> #{c[x]}"
            if p[x] == '.'
              abort('Logic error: Too many balls')
            else
             # ball must be added
              add_pos = [x, y]
              break
            end
          end
        end
        if add_pos
          break
        end
      end
      if add_pos
        fetch_from_storage(index, add_pos[0], add_pos[1])
        prev_dots[add_pos[1]][add_pos[0]] = '.'
        dump(prev_dots)
      end
    end until !add_pos
  end
end

def goto(x, y)
    puts "Go to #{x}, #{y}"
    s = "M #{x.to_i()} #{y.to_i()}"
    $sp.puts s
    wait_response(s)
end

def set_parameters
  # Origin
  #s = "o -465 570"
  s = "o -440 #{530+0.15*400}"
  $sp.puts s
  wait_response(s)
  # servo_delay pickup_hi_delay magnet_off_delay angle_up angle_down
  # Defaults 250 250 100 30 75
  s = "s 250 350 100 30 75"
  $sp.puts s
  wait_response(s)
  # Magnet power
  # If HIGH is 255, we pick up multiple balls
  # If LOW is below 80, we drop the ball when moving
  s = "w 200 100"
  $sp.puts s
  wait_response(s)
end

def move_test1()
  set_parameters()
  goto(0, 0)
  sleep 1
  s = "P"
  $sp.puts s
  wait_response(s)
  count = 1
  while true
    s = "D"
    $sp.puts s
    wait_response(s)
    sleep 2
    s = "P"
    $sp.puts s
    wait_response(s)
    if true
    goto(25, 0)
    s = "D"
    $sp.puts s
    wait_response(s)
    if count < 5
      sleep 2
      count = count + 1
    else
      puts "Resetting"
      count = 1
      s = "R"
      $sp.puts s
      wait_response(s)
      goto(25, 0)
    end
    s = "P"
    $sp.puts s
    wait_response(s)
    goto(25, 12)
    s = "D"
    $sp.puts s
    wait_response(s)
    sleep 2
    s = "P"
    $sp.puts s
    wait_response(s)
    goto(0, 12)
    s = "D"
    $sp.puts s
    wait_response(s)
    sleep 2
    s = "P"
    $sp.puts s
    wait_response(s)
    goto(0, 0)
    sleep 1
    end
  end
end

def move_test()
  count = 0
  while true
    s = "M 0 5 25 5"
    verbose "> #{s}"
    $sp.puts s
    wait_response(s)
    count = count + 1
    puts("Moved #{count} times")
    sleep 0.2
    s = "M 25 5 0 5"
    verbose "> #{s}"
    $sp.puts s
    wait_response(s)
    count = count + 1
    puts("Moved #{count} times")
    sleep 0.2
  end
end

$sp = nil
if !$dry_run
  portnum = 0
  if ARGV.size > 0
    portnum = ARGV[0].to_i
  end
  $sp = SerialPort.new("/dev/ttyUSB#{portnum}",
                       { 'baud' => 57600,
                         'data_bits' => 8,
                         'parity' => SerialPort::NONE
                       })
  begin
    begin
      line = $sp.gets
    end while !line || line.empty?
    line.strip!
  end while !line.start_with? "Ball Clock ready"
  
  if do_reset
    $sp.puts('R')
    Process.exit
  end
  if do_goto
    puts("Go to #{goto_x} #{goto_y}")
    $sp.puts("M #{goto_x} #{goto_y}")
    Process.exit
  end
  set_parameters()
  if do_move_test
    move_test1()
    Process.exit
  end
  if replay_file
    File.readlines(replay_file).each do |line|
      line.strip!
      $sp.flush_input
      $sp.puts line
      wait_response(line)
    end
    Process.exit
  end
end

tz = ENV['TZ']
ENV['TZ'] = 'UTC'
prev_time = Time.new(2000, 1, 1, 0, 0, 0)
ENV['TZ'] = tz
prev = prev_time.strftime("%H%M")

if initial_time != ''
  prev = initial_time
end

if storage_count > 0
  index = 0
  offset = 0
  for i in 0..storage_count-1
    $storage[index][i-offset] = 1
    if i-offset >= 4
      offset += 5
      index = index +1
    end
  end
end

first = true
done = false
abort_count = 0

while true
  if run_fast
    if first
      current_time = DateTime.new(1970, 1, 1, 0, 0, 0).to_time
    else
      current_time = current_time + 1
    end
  else
    current_time = DateTime.now.to_time
  end
  current = current_time.strftime("%H%M")
  verbose current
  if current != prev
    puts "#{prev} to #{current}"
    if !$dry_run
      $sp.flush_input
      $sp.puts "R"
      wait_response("R")
      $sp.puts "E 0"
      wait_response("E 0")
    end
    for i in 0..3
      if current[i] != prev[i]
        verbose "#{prev[i]} to #{current[i]}"
        change(i, prev[i].to_i, current[i].to_i)
      end
    end
    if !$dry_run
      $sp.flush_input
      $sp.puts "M 35 0"
      wait_response("M 35 0")
      $sp.puts "E 1"
      wait_response("E 1")
    end
    prev = current
    `python showtime.py #{current[0]}#{current[1]}:#{current[2]}#{current[3]}R`
  end
  if run_fast
    if !first && (current == "0000")
      puts "DONE"
      Process.exit
    end
    first = false
  else
    sleep(0.5)
  end
  check_abort()
end
