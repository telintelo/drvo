DEV=$(ls /dev/opdracht3-* | head -n 1)

echo "Using device $DEV"

echo -e "\nReading from device:"
head -n 5 $DEV
echo -e "Done."

echo -e "\nWriting to device:"
echo "Hello from user-space!" > $DEV
echo -e "Done. Kernel buffer:"
sudo dmesg | tail -n 8
echo -e "Done."
