request = function()
  if math.random() < 0.8 then
    return wrk.format("GET", "/")
  else
    return wrk.format("GET", "/large_file")
  end
end
