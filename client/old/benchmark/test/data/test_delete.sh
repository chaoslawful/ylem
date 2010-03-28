
if [ ! -d delete ]; then
  mkdir delete
fi

i=0
while [ $i -lt 5 ]; do
  echo $i;
  ../test_delete $i &
  let i=$i+1
done

