local i = 0 -- Initialize index

request = function()
  i = i + 1 -- Increment index
  if i % 9 == 0 then
    -- Every 5th request
    return wrk.format("GET", "/large_file")
  elseif i % 11 == 0 then
    -- Every 5th request
    return wrk.format("GET", "/large_file2")
  else
    -- All other requests
    return wrk.format("GET", "/")
  end
end
