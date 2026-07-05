sudo apt update && sudo apt upgrade
sudo apt install -y nano git vim netplan.io neofetch device-tree-compiler python3 v4l-utils tree tcpdump

sudo pip3 install -U jetson-stats
sudo systemctl restart jtop.service

sudo cp 99-quac-network-manager.yaml /etc/netplan/99-quac-network-manager.yaml
sudo netplan generate
sudo netplan apply

sudo cp 99-quac.rules /etc/udev/rules.d/99-quac.rules
sudo udevadm control --reload-rules
sudo udevadm trigger