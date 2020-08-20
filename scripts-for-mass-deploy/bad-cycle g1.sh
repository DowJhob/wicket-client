while true 
do
for IP in 'root@10.1.7.220' 'root@10.1.7.210'
do
echo "=============================== COPY to SERVER ========================"
pscp -t 3 -v -H $IP /home/a/Desktop/attempt-threaded-wicket-gate-client/build-r-Debug/r /usr/r
echo "==================================== SYNC ============================="
pssh -t 1 -v -i -H $IP sync
#echo "================================ COPY to RUN =========================="
#pssh -p 32 -t 4 -v -i -H $IP cp /usr/r /run/r
#echo "================================= KILLALL R ==========================="
#pssh -p 32 -t 4 -v -i -H $IP killall r
#echo "==================================== RUN =============================="
#pssh -p 32 -t 4 -v -i -H $IP /run/r &
done
done
