path='/home/a/Documents/GitHub/wicket-client/scripts-for-mass-deploy'
path2='/home/a/Documents/GitHub/wicket-client/build'
HOST_LIST=$path'/host-listS/host_list-GATE-3.txt'
echo "=============================== COPY to SERVER ========================"
pscp -p 32 -t 4 -v -h $HOST_LIST $path2/r /usr/r
echo "==================================== SYNC ============================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST sync
echo "================================ COPY to RUN =========================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST cp /usr/r /run/r
echo "================================= KILLALL R ==========================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST killall r
echo "==================================== RUN =============================="
pssh -p 32 -t 4 -v -i -h $HOST_LIST /run/r &
