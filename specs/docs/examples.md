# Example Specifications

This will contain some example specifications that accomplish something interesting

## PI Estimate

This specification estimates the value of PI.  It totally ignores the input records, but for each input record, the result gets more accurate. It doesn't get very accurate: You get about as many significant digits as you have in the number of input records, so processing a million records will get you less than 6 significant digits of PI -- more of less 3.141
```
  WORD 1 .                       # Just to make the records flow
  SET  "#0:=2*(frand()-0.5)"     # #0 is a random value between -1 and 1
  SET  "#1:=2*(frand()-0.5)"     # #1 is also random value between -1 and 1 
  SET  "#2:=pow(#0,2)+pow(#1,2)" # The distance of the (#0,#1) point from the origin, squared.
  SET  "#4:=#4+1"                # #4 just counts the records.
  IF   "#2<=1.0" THEN            # if the (#0,#1) point is within the unit circle...
     SET  "#3:=#3+1"             #    increment counter #3
  ENDIF 
EOF 
  PRINT  "4*#3/#4" 20            # The proportion of points within the unit circle multiplied 
                                 # by 4 is a good estimate of PI
```

## Random Check

This specification doesn't need any input. It generates 10,000 random numbers between 0 and 9, and counts how may of them are 7. The expected value is, of course, 1000, and the standard deviation should be 30. This specification returns `OK` if the number of occurrences of 7 is within two standard deviations from the expected value. 
```
WHILE '#0<10000' DO
   PRINT 'fmap_sample(a,rand(10))' .
   SET '#0+=1'
DONE
SET '#1:=fmap_count(a,7)'
IF '#1 > 940 & #1 < 1060' THEN
   /OK/ 1
ELSE
   /NOT OK/ 1
ENDIF
```
