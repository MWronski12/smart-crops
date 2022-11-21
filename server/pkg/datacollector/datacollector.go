package datacollector

import (
	"encoding/json"
	"log"

	"server-poc/pkg/mqtt"
	"server-poc/pkg/sensordata"

	"gorm.io/gorm"
)

type NotificationHandler func(sensordata.SensorData)
type Service interface {
	ListenForNewData(NotificationHandler)
}

// Internal structure for keeping all data and dependencies
type service struct {
	db         *gorm.DB
	mqttClient mqtt.Client
	observers  []NotificationHandler
}

func Start(mqttClient mqtt.Client, db *gorm.DB) (Service, error) {
	s := service{
		db:         db,
		mqttClient: mqttClient,
	}
	if err := mqttClient.Sub("sensors/*", 1, s.handleData); err != nil {
		return nil, err
	}
	return &s, nil
}

func (s *service) ListenForNewData(handler NotificationHandler) {
	// Add new observer to the list
	s.observers = append(s.observers, handler)
}

func (s *service) handleData(msg mqtt.Msg) {
	type MessagePayload struct {
		Temp     float64 `json:"temp"`
		Pres     float64 `json:"pres"`
		Light    float64 `json:"light"`
		SensorID int     `json:"sensorId"`
	}

	// Decode the payload
	var payload MessagePayload
	if err := json.Unmarshal(msg.Payload(), &payload); err != nil {
		log.Println("Received malformed sensordata:", err)
		return
	}

	// Add data to the database
	data := sensordata.SensorData{
		Temperature:   payload.Temp,
		Pressure:      payload.Pres,
		LightIntesity: payload.Light,
		SensorID:      payload.SensorID,
	}
	if err := data.Save(s.db); err != nil {
		log.Println("Failed to save sensor data to the database:", err)
		return
	}

	// Notify all observers
	for _, observer := range s.observers {
		go observer(data)
	}
}
