# Server PoC

## Development
To run this service simply run:
```bash
go run .
```
To push a new container version to docker-hub run:
```bash
docker compose build
docker compose push
```

## Deployment
To deploy this application run the following commands on your server:
```bash
git clone https://github.com/SmartCrops/server-poc.git
cd server-poc
docker compose pull
docker compose up
```
Deployment includes [watchtower](https://github.com/containrrr/watchtower) to make container autmatically update based on dockerhub.