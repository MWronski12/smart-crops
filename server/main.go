package main

import (
	"log"
	"time"

	"server-poc/pkg/datacollector"
	"server-poc/pkg/mobileapi"
	"server-poc/pkg/mqtt"
	"server-poc/pkg/sensordata"
	"server-poc/pkg/waterplanner"

	"github.com/glebarez/sqlite"
	"gorm.io/gorm"
)

const (
	mqttURL      = "tcp://172.111.242.63:6666"
	mqttUsername = "roslina"
	mqttPassword = "smartcrops"
	dbPath       = "artifacts/baza.db"
	apiPort      = "8080"
	restartTime  = time.Second * 2
)

func run() error {
	log.Println("Initializing database...")
	db, err := gorm.Open(sqlite.Open(dbPath), &gorm.Config{})
	if err != nil {
		return err
	}

	log.Println("Performing database migrations...")
	err = db.AutoMigrate(sensordata.SensorData{})
	if err != nil {
		return err
	}

	log.Println("Initializing mqtt...")
	mqttClient, err := mqtt.Connect(mqttURL, mqttUsername, mqttPassword)
	if err != nil {
		return err
	}

	log.Println("Initializing datacollector service...")
	datacollectorService, err := datacollector.Start(mqttClient, db)
	if err != nil {
		return err
	}

	log.Println("Initializing waterplanner service...")
	waterplanner.Start(db, mqttClient, datacollectorService)

	log.Println("Running mobile api on port", apiPort)
	if err := mobileapi.Run(db, apiPort); err != nil {
		return err
	}

	return nil
}

func main() {
	for {
		err := run()
		log.Println("Run function exited:", err)
		log.Println("Scheduling next startup in", restartTime)
		time.Sleep(restartTime)
	}
}
