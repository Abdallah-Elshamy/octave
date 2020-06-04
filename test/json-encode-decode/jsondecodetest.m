% test jsondecode

%% Test 1: decode null values

% null, in nonnumeric arrays to Empty double []
json = '["str", 5, null, true]';
exp  = {'str'; 5; []; true};
act  = jsondecode (json);
assert (isequal (exp, act));

% null, in numeric arrays to NaN
json = '[1, 2, null, 3]';
exp  = [1; 2; NaN; 3];
act  = jsondecode (json);
assert (isequaln (exp, act));

% corner case: array of null values
json = '[null, null, null]';
exp  = [NaN; NaN; NaN];
act  = jsondecode (json);
assert (isequaln (exp, act));

%% Test 2: decode Boolean, Number and String values

assert (jsondecode ('true'));
assert (~ jsondecode ('false'));

act  = jsondecode ('123.45');
assert (isequal (123.45, act));

act  = jsondecode ('"hello there"');
assert (isequal ('hello there', act));
