% test jsonencode

% Note: This script is intended to be a script-based unit test
%       for MATLAB to test compatibility.  Don't break that!

%% Test 1: encode logical and numeric scalars, NaN and Inf
assert (isequal (jsonencode (true), 'true'));
assert (isequal (jsonencode (50.025), '50.025'));
assert (isequal (jsonencode (NaN), 'null'));
assert (isequal (jsonencode (Inf), 'null'));
assert (isequal (jsonencode (-Inf), 'null'));

% Customized encoding of Nan, Inf, -Inf
assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', true), 'null'));
assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', true), 'null'));
assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', true), 'null'));

assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', false), 'NaN'));
assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', false), 'Infinity'));
assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', false), '-Infinity'));

%% Test 2: encode character vectors and arrays
assert (isequal (jsonencode (''), '""'));
assert (isequal (jsonencode ('hello there'), '"hello there"'));
assert (isequal (jsonencode (['foo'; 'bar']), '["foo","bar"]'));
assert (isequal (jsonencode (['foo', 'bar'; 'foo', 'bar']), '["foobar","foobar"]'));

data = [[['foo'; 'bar']; ['foo'; 'bar']], [['foo'; 'bar']; ['foo'; 'bar']]];
exp  = '["foofoo","barbar","foofoo","barbar"]';
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 3: encode numeric and logical arrays (with NaN and Inf)
% test simple vectors
assert (isequal (jsonencode ([]), '[]'));
assert (isequal (jsonencode ([1, 2, 3, 4]), '[1,2,3,4]'));
assert (isequal (jsonencode ([true; false; true]), '[true,false,true]'));

% test arrays
data = [1 NaN; 3 4];
exp  = '[[1,null],[3,4]]';
act  = jsonencode (data);
assert (isequal (exp, act));

data = cat (3, [NaN, 3; 5, Inf], [2, 4; -Inf, 8]);
exp  = '[[[null,2],[3,4]],[[5,null],[null,8]]]';
act  = jsonencode (data);
assert (isequal (exp, act));

% Customized encoding of Nan, Inf, -Inf
data = cat (3, [1, NaN; 5, 7], [2, Inf; 6, -Inf]);
exp  = '[[[1,2],[NaN,Infinity]],[[5,6],[7,-Infinity]]]';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

data = [true false; true false; true false];
exp  = '[[true,false],[true,false],[true,false]]';
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 4: encode containers.Map

% KeyType must be char to encode objects of containers.Map
assert (isequal (jsonencode (containers.Map('1', [1, 2, 3])), '{"1":[1,2,3]}'));

data = containers.Map({'foo'; 'bar'; 'baz'}, [1, 2, 3]);
exp  = '{"bar":2,"baz":3,"foo":1}';
act  = jsonencode (data);
assert (isequal (exp, act));

data = containers.Map({'foo'; 'bar'; 'baz'}, {{1, 'hello', NaN}, true, [2, 3, 4]});
exp  = '{"bar":true,"baz":[2,3,4],"foo":[1,"hello",NaN]}';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

%% Test 5: encode scalar structs
% check the encoding of Boolean, Number and String values inside a struct
data = struct ('number', 3.14, 'string', 'string', 'boolean', false);
exp  = '{"number":3.14,"string":"string","boolean":false}';
act  = jsonencode (data);
assert (isequal (exp, act));

% check the decoding of null, Inf and -Inf values inside a struct
data = struct ('numericArray', [7, NaN, Inf, -Inf]);
exp  = '{"numericArray":[7,null,null,null]}';
act  = jsonencode (data);
assert (isequal (exp, act));

% Customized encoding of Nan, Inf, -Inf
exp  = '{"numericArray":[7,NaN,Infinity,-Infinity]}';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

% check the encoding of structs inside a struct
data = struct ('object', struct ('field1', 1, 'field2', 2, 'field3', 3));
exp  = '{"object":{"field1":1,"field2":2,"field3":3}}';
act  = jsonencode (data);
assert (isequal (exp, act));

% check the encoding of empty structs, empty arrays and Inf inside a struct
data = struct ('a', Inf, 'b', [], 'c', struct ());
exp  = '{"a":null,"b":[],"c":{}}';
act  = jsonencode (data);
assert (isequal (exp, act));

% a big test
tmp1 = struct ('para', ['A meta-markup language, used to create markup languages', ...
               ' such as DocBook.'],'GlossSeeAlso', {{'GML'; 'XML'}});
tmp2 = struct ('ID', 'SGML', 'SortAs', 'SGML', 'GlossTerm', ...
               'Standard Generalized Markup Language', 'Acronym', 'SGML', ...
               'Abbrev', 'ISO 8879:1986', 'GlossDef', tmp1, 'GlossSee', 'markup');
data = struct ('glossary', struct ('title', 'example glossary', 'GlossDiv', ...
                struct ('title', 'S', 'GlossList', struct ('GlossEntry', tmp2))));
exp = ['{' , ...
    '"glossary":{', ...
        '"title":"example glossary",', ...
		'"GlossDiv":{', ...
            '"title":"S",', ...
			'"GlossList":{', ...
                '"GlossEntry":{', ...
                    '"ID":"SGML",', ...
					'"SortAs":"SGML",', ...
					'"GlossTerm":"Standard Generalized Markup Language",', ...
					'"Acronym":"SGML",', ...
					'"Abbrev":"ISO 8879:1986",', ...
					'"GlossDef":{', ...
                        '"para":"A meta-markup language, ', ...
                        'used to create markup languages such as DocBook.",', ...
						'"GlossSeeAlso":["GML","XML"]', ...
                    '},', ...
					'"GlossSee":"markup"', ...
                '}', ...
            '}', ...
        '}', ...
    '}', ...
'}'];
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 6: encode struct arrays
data = struct ('structarray', struct ('a', {1; 3}, 'b', {2; 4}));
exp  = '{"structarray":[{"a":1,"b":2},{"a":3,"b":4}]}';
act  = jsonencode (data);
assert (isequal (exp, act));

% a big Test
tmp1 = struct ('id', {0; 1; 2}, 'name', {'Collins'; 'Hays'; 'Griffin'});
tmp2 = struct ('id', {0; 1; 2}, 'name', {'Osborn'; 'Mcdowell'; 'Jewel'});
tmp3 = struct ('id', {0; 1; 2}, 'name', {'Socorro'; 'Darla'; 'Leanne'});
data = struct ('x_id', {'5ee28980fc9ab3'; '5ee28980dd7250'; '5ee289802422ac'}, ...
               'index', {0; 1; 2}, 'guid', {'b229d1de-f94a'; '39cee338-01fb'; '3db8d55a-663e'}, ...
               'latitude', {-17.124067; 13.205994; -35.453456}, 'longitude', ...
               {-61.161831; -37.276231; 14.080287}, 'friends', {tmp1; tmp2; tmp3});
exp  = ['[', ...
  '{', ...
    '"x_id":"5ee28980fc9ab3",', ...
    '"index":0,', ...
    '"guid":"b229d1de-f94a",', ...
    '"latitude":-17.124067,', ...
    '"longitude":-61.161831,', ...
    '"friends":[', ...
      '{', ...
        '"id":0,', ...
        '"name":"Collins"', ...
      '},', ...
      '{', ...
        '"id":1,', ...
        '"name":"Hays"', ...
      '},', ...
      '{', ...
        '"id":2,', ...
        '"name":"Griffin"', ...
      '}', ...
    ']', ...
  '},', ...
  '{', ...
    '"x_id":"5ee28980dd7250",', ...
    '"index":1,', ...
    '"guid":"39cee338-01fb",', ...
    '"latitude":13.205994,', ...
    '"longitude":-37.276231,', ...
    '"friends":[', ...
      '{', ...
        '"id":0,', ...
        '"name":"Osborn"', ...
      '},', ...
      '{', ...
        '"id":1,', ...
        '"name":"Mcdowell"', ...
      '},', ...
      '{', ...
        '"id":2,', ...
        '"name":"Jewel"', ...
      '}', ...
    ']', ...
  '},', ...
  '{', ...
    '"x_id":"5ee289802422ac",', ...
    '"index":2,', ...
    '"guid":"3db8d55a-663e",', ...
    '"latitude":-35.453456,', ...
    '"longitude":14.080287,', ...
    '"friends":[', ...
      '{', ...
        '"id":0,', ...
        '"name":"Socorro"', ...
      '},', ...
      '{', ...
        '"id":1,', ...
        '"name":"Darla"', ...
      '},', ...
      '{', ...
        '"id":2,', ...
        '"name":"Leanne"', ...
      '}', ...
    ']', ...
  '}', ...
']'];
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 7: encode cell arrays
assert (isequal ('[]', jsonencode ({})));
assert (isequal ('[5]', jsonencode ({5})));
assert (isequal ('["hello there"]', jsonencode ({'hello there'})));

data = {'true', 'true'; 'false', 'true'};
exp  = '["true","false","true","true"]';
act  = jsonencode (data);
assert (isequal (exp, act));

data = {'foo'; 'bar'; {'foo'; 'bar'}};
exp  = '["foo","bar",["foo","bar"]]';
act  = jsonencode (data);
assert (isequal (exp, act));

% cell array of structs & a big test
tmp1 =  struct ('x_id', '5ee28980fc9ab3', 'index', 0, 'guid', 'b229d1de-f94a', ...
                'latitude', -17.124067, 'longitude', -61.161831, 'friends', ...
               struct ('id', {0; 1; 2}, 'name', {'Collins'; 'Hays'; 'Griffin'}));
tmp2 = struct ('numericArray',{{'str'; 5; []}}, 'nonnumericArray', {[1; 2; NaN]});
tmp3 = struct ('firstName', 'John','lastName', 'Smith', 'age', 25, 'address', ...
               struct('streetAddress', '21 2nd Street', 'city', 'New York', 'state', 'NY'), ...
               'phoneNumber', struct ('type', 'home', 'number', '212 555-1234'));
data = {tmp1; tmp2; tmp3};
exp  = ['[', ...
  '{', ...
    '"x_id":"5ee28980fc9ab3",', ...
    '"index":0,', ...
    '"guid":"b229d1de-f94a",', ...
    '"latitude":-17.124067,', ...
    '"longitude":-61.161831,', ...
    '"friends":[', ...
      '{', ...
        '"id":0,', ...
        '"name":"Collins"', ...
      '},', ...
      '{', ...
        '"id":1,', ...
        '"name":"Hays"', ...
      '},', ...
      '{', ...
        '"id":2,', ...
        '"name":"Griffin"', ...
      '}', ...
    ']', ...
  '},', ...
  '{"numericArray":["str",5,[]],"nonnumericArray":[1,2,null]},', ...
  '{', ...
     '"firstName":"John",', ...
     '"lastName":"Smith",', ...
     '"age":25,', ...
     '"address":', ...
     '{', ...
         '"streetAddress":"21 2nd Street",', ...
         '"city":"New York",', ...
         '"state":"NY"', ...
     '},', ...
     '"phoneNumber":', ...
         '{', ...
           '"type":"home",', ...
           '"number":"212 555-1234"', ...
         '}', ...
 '}]'];
act  = jsonencode (data);
assert (isequal (exp, act));

% cell array of diferrent types & Customized encoding of Nan, Inf, -Inf
tmp =  struct ('x_id', '5ee28980dd7250', 'index', 1, 'guid', '39cee338-01fb', ...
                'latitude', 13.205994, 'longitude', -37.276231, 'friends', ...
                struct ('id', {0; 1; 2}, 'name', {'Osborn'; 'Mcdowell'; 'Jewel'}));
data = {NaN; true; Inf; 2531.023; 'hello there'; tmp};
exp  = ['[NaN,true,Infinity,2531.023,"hello there",', ...
  '{', ...
    '"x_id":"5ee28980dd7250",', ...
    '"index":1,', ...
    '"guid":"39cee338-01fb",', ...
    '"latitude":13.205994,', ...
    '"longitude":-37.276231,', ...
    '"friends":[', ...
      '{', ...
        '"id":0,', ...
        '"name":"Osborn"', ...
      '},', ...
      '{', ...
        '"id":1,', ...
        '"name":"Mcdowell"', ...
      '},', ...
      '{', ...
        '"id":2,', ...
        '"name":"Jewel"', ...
      '}', ...
    ']', ...
  '}]'];
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

% a big example
tmp1 = struct ('x_id', '5ee28980fc9ab3', 'index', 0, 'guid', 'b229d1de-f94a', ...
               'latitude', -17.124067, 'longitude', -61.161831, 'friends', ...
               struct ('id', {0; 1; 2}, 'name', {'Collins'; 'Hays'; 'Griffin'}));
tmp2 = struct ('numericArray',{{'str'; 5; -Inf}}, 'nonnumericArray', {[1; 2; NaN]});
tmp3 = struct ('firstName', 'John','lastName', 'Smith', 'age', 25, 'address', ...
                  struct('streetAddress', '21 2nd Street', 'city', 'New York', 'state', 'NY'), ...
                  'phoneNumber', struct ('type', 'home', 'number', '212 555-1234'));
data = {{'str'; Inf; {}}; [1; 2; NaN]; {'foo'; 'bar'; {'foo'; 'bar'}};
       cat(3, [1, 3; 5, 7], [2, 4; 6, 8]); {tmp1; tmp2 ;tmp3}};
exp  = ['[["str",null,[]],[1,2,null],["foo","bar",["foo","bar"]],', ...
  '[[[1,2],[3,4]],[[5,6],[7,8]]],' , ...
  '[', ...
    '{', ...
      '"x_id":"5ee28980fc9ab3",', ...
      '"index":0,', ...
      '"guid":"b229d1de-f94a",', ...
      '"latitude":-17.124067,', ...
      '"longitude":-61.161831,', ...
      '"friends":[', ...
        '{', ...
          '"id":0,', ...
          '"name":"Collins"', ...
        '},', ...
        '{', ...
          '"id":1,', ...
          '"name":"Hays"', ...
        '},', ...
        '{', ...
          '"id":2,', ...
          '"name":"Griffin"', ...
        '}', ...
      ']', ...
    '},', ...
    '{"numericArray":["str",5,null],"nonnumericArray":[1,2,null]},', ...
    '{', ...
       '"firstName":"John",', ...
       '"lastName":"Smith",', ...
       '"age":25,', ...
       '"address":', ...
       '{', ...
           '"streetAddress":"21 2nd Street",', ...
           '"city":"New York",', ...
           '"state":"NY"', ...
       '},', ...
       '"phoneNumber":', ...
           '{', ...
             '"type":"home",', ...
             '"number":"212 555-1234"', ...
           '}', ...
   '}]]'];
act  = jsonencode (data);
assert (isequal (exp, act));
