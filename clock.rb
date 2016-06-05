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

ROWS = 7
COLUMNS = 5

def debug(s)
  #puts(s)
end

def dump(dots)
  puts "-----"
  for i in 0..ROWS-1
    puts dots[i]
  end
  puts "-----"
end

def get_matrix(digit)
  a = []
  bits = @dots[digit]
  for i in 0..ROWS-1
    a[i] = bits[i*COLUMNS..(i+1)*COLUMNS-1]
  end
  return a
end

def move_ball(index,
              from_x, from_y,
              to_x, to_y)
  puts("-- #{index}: Move ball (#{from_x}, #{from_y}) to (#{to_x}, #{to_y})")
end

def move_to_storage(index,
                    from_x, from_y)
  puts("-- #{index}: Move ball (#{from_x}, #{from_y}) to storage")
end

def fetch_from_storage(index,
                       to_x, to_y)
  puts("-- #{index}: Move ball from storage to (#{to_x}, #{to_y})")
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
    end
  end until !moved
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
        puts "#{prev} to #{current}"
        change(i, prev[i].to_i, current[i].to_i)
        done = true
      end
    end
    prev = current
  end
  sleep(0.5)
end
