from golang:alpine as builder
workdir /build
run apk add build-base
copy . .
run go build .
run mkdir /app
run mv ./server-poc /app/server-poc

from alpine
workdir /app
expose 8080
copy --from=builder /app /app

entrypoint /app/server-poc