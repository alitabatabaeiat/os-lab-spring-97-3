sudo rm -rf /boot/*-3.16.0* &&
make -j $(nproc) &&
sudo make -j $(nproc) modules_install &&
sudo make -j $(nproc) install &&
sudo shutdown -r now
