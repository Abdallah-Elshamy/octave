ts = localtime (time ())
(isstruct (ts)
 && struct_contains (ts, "usec")
 && struct_contains (ts, "year")
 && struct_contains (ts, "mon")
 && struct_contains (ts, "mday")
 && struct_contains (ts, "sec")
 && struct_contains (ts, "min")
 && struct_contains (ts, "wday")
 && struct_contains (ts, "hour")
 && struct_contains (ts, "isdst")
 && struct_contains (ts, "yday"))
