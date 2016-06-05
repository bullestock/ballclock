require 'date'

one   = '  .   ..  . .    .    .    .  .....'
two   = ' ... .   .    .  ..  .   .    .....'
three = ' ... .   .    .  ..     ..   . ... '
four  = '   .   ..  . . .  . .....   .    . '
five  = '......    ....     .    ..   . ... '
six   = '  ..  .   .    .... .   ..   . ... '
seven = '.....    .   .   .   .    .    .   '
eight = ' ... .   ..   . ... .   ..   . ... '
nine  = ' ... .   ..   . ....    .   .  ..  '
zero  = ' ... .   ..   ..   ..   ..   . ... '
@dots = [ zero, one, two, three, four, five, six, seven, eight, nine ]

def debug(s)
  #puts(s)
end

def dump(dots)
  for i in 0..6
    puts dots[i*5..(i+1)*5-1]
  end
end

def show(digit)
  dump(@dots[digit])
end

def get_matrix(digit)
  a = []
  bits = @dots[digit]
  for i in 0..6
    a[i] = bits[i*5..(i+1)*5-1]
  end
  return a
end

def change(index, prev, cur)
  puts "Change #{index} from #{prev} to #{cur}"
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
  begin
    add_pos = nil
    rem_pos = nil
    moved = false
    for y in 0..6
      p = prev_dots[y]
      c = cur_dots[y]
      for x in 0..4
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
      prev_dots[add_pos[1]][add_pos[0]] = '.'
      prev_dots[rem_pos[1]][rem_pos[0]] = ' '
      dump(prev_dots)
      moved = true
    end
  end until !moved
end

# for i in 0..9
#   puts get_matrix(i)
# end

#t = DateTime.now.to_time.strftime("%H%M")
prev = DateTime.now.to_time.strftime("%M%S")

done = false
while !done
  current = DateTime.now.to_time.strftime("%M%S")
  if current != prev
    for i in 0..3
      if current[i] != prev[i]
        change(i, prev[i].to_i, current[i].to_i)
        done = true
      end
    end
    prev = current
  end
  sleep(0.5)
end
