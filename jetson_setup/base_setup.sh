sudo apt update && sudo apt upgrade
sudo apt install -y nano git vim netplan.io neofetch

sudo mv 01-network-manager.yaml /etc/netplan/01-network-manager.yaml
sudo netplan generate
sudo netplan apply