package sensordata

import "gorm.io/gorm"

type SensorData struct {
	gorm.Model
	Temperature   float64
	Pressure      float64
	LightIntesity float64
	SensorID      int
}

func GetBySensorID(db *gorm.DB, sensorID int) ([]SensorData, error) {
	var data []SensorData
	err := db.Where(&SensorData{SensorID: sensorID}).Find(&data).Error
	return data, err
}

func (data SensorData) Save(db *gorm.DB) error {
	return db.Save(data).Error
}
