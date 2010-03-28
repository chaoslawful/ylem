
i=0

if [ ! -d upload ]; then
  mkdir upload
fi

while [ $i -lt 5 ]; do
  echo $i;
  ../test_upload $i &
  let i=$i+1
done

