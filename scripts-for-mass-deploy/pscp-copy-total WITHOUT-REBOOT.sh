HOST_LIST='/home/a/Desktop/attempt-threaded-wicket-gate-client/host-listS/host_list2.txt'
echo "=============================== COPY to SERVER ========================"
pscp -p 32 -t 4 -v -h $HOST_LIST /home/a/Desktop/attempt-threaded-wicket-gate-client/build-r-Debug/r /usr/r
echo "==================================== SYNC ============================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST sync
echo "================================ COPY to RUN =========================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST cp /usr/r /run/r
echo "================================= KILLALL R ==========================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST killall r
echo "==================================== RUN =============================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST /run/r &
