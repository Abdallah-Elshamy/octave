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

%% Test 3: decode Array of Booleans, Numbers and Strings values

json = '[true, true, false, true]';
exp  = [1; 1; 0; 1];
act  = jsondecode (json);
assert (isequal (exp, act));

json = '["true", "true", "false", "true"]';
exp  = {'true'; 'true'; 'false'; 'true'};
act  = jsondecode (json);
assert (isequal (exp, act));

json = '["foo", "bar", ["foo", "bar"]]';
exp  = {'foo'; 'bar'; {'foo'; 'bar'}};
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[15000, 5, 12.25, 1502302.3012]';
exp  = [15000; 5; 12.25; 1502302.3012];
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[[1,2]]';
exp  = [1 2];
act  = jsondecode (json);
assert (isequal (exp, act));

% If they have the same dimensions -> transform to an array
json = '[[1, 2], [3, 4]]';
exp  = [1 2; 3 4];
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[[[1, 2], [3, 4]], [[5, 6], [7, 8]]]';
exp  = cat (3, [1, 3; 5, 7], [2, 4; 6, 8]);
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[[true, false], [true, false], [true, false]]';
exp  = [1 0; 1 0; 1 0];
act  = jsondecode (json);
assert (isequal (exp, act));

% If they have different dimensions -> transform to a cell array
json = '[[1, 2], [3, 4, 5]]';
exp  = {[1; 2]; [3; 4; 5]};
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[1, 2, [3, 4]]';
exp  = {1; 2; [3; 4]};
act  = jsondecode (json);
assert (isequal (exp, act));

json = '[true, false, [true, false, false]]';
exp  = {1; 0; [1; 0; 0]};
act  = jsondecode (json);
assert (isequal (exp, act));
