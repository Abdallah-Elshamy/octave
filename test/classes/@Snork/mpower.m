function s = mpower(s1, x)

  if ~isa(s1, 'Snork') || isa(x, 'Snork')
    error('mpower Snork!!!');
  end

  s = s1;
  s.gick = s.gick ^ x;

end
