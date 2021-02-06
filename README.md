
chmod +x ./build_and_upload.sh
export SSHPASS='%ssh_remote_password%'

It's used dashamail host, so add it to ~/.ssh/config:
```
Host dashamail
        User %ssh_user%
        Hostname %ssh_host%
        Port %ssh_port%
```

IMAGE_NAME, IMAGE_TAG, SERVER_NAME variables are declared in bash script in arrays PODS and HOSTS

./build_and_upload.sh

