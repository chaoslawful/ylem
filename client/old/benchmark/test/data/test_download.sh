
if [ ! -d download ]; then
  mkdir download
fi

i=0
while [ $i -lt 10 ]; do
  echo $i;
  ../test_download $i &
  let i=$i+1
done

