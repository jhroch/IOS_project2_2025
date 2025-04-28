#!/bin/bash
# IOS 2025 - kontrolni skript pro syntaktickou spravnost vystupu
# Pouziti: ./kontrola-vystupu.sh < proj2.out
# Pripadne chyby piste do Forma predmetu na moodle.vut.cz

tmp1=`mktemp`
tmp2=`mktemp`

cat > $tmp1
echo "--------------------------------------"
echo "Chybi nasledujici typy radek:"

#---------------
Q="^[1-9][0-9]*: P: started$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: started"
fi

cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: started$"

grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: started"
fi

cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: started$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: started"
fi

cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: P: arrived to 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: arrived to 0"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: P: arrived to 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: arrived to 1"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: P: leaving 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: leaving 0"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: P: leaving 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: leaving 1"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: P: finish$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: P: finish"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: arrived to 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: arrived to 0 (Zadne osobni auto neprijelo do pristavu 0)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: arrived to 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: arrived to 0 (Zadne nakladni auto neprijelo do pristavu 0)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: arrived to 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: arrived to 1 (Zadne osobni auto neprijelo do pristavu 1)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: arrived to 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: arrived to 1 (Zadne nakladni auto neprijelo do pristavu 1)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: boarding$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: boarding (Zadne osobni auto se nenalodilo)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: boarding$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: boarding (Zadne nakladni auto se nenalodilo)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: leaving in 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: leaving in 0 (Zadne osobni auto se nevylodilo v pristavu 0)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: leaving in 0$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: leaving in 0 (Zadne nakladni auto se nevylodilov pristavu 0)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: O [1-9][0-9]*: leaving in 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: O idO: leaving in 1 (Zadne osobni auto se nevylodilo v pristavu 1)"
fi
cat < $tmp2 >$tmp1
#---------------
Q="^[1-9][0-9]*: N [1-9][0-9]*: leaving in 1$"
grep -v "$Q" <$tmp1 >$tmp2

if grep "$Q" < $tmp1 > /dev/null; 
then true
else echo "A: N idN: leaving in 1 (Zadne nakladni auto se nevylodilo v pristavu 1)"
fi
cat < $tmp2 >$tmp1
#---------------



echo "--------------------------------------"
echo "Nasledujci radky jsou syntakticky chybne, nebo ve vystupu prebyvaji:"

cat < $tmp1
