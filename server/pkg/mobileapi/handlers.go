package mobileapi

import (
	"net/http"
	"server-poc/pkg/sensordata"
	"strconv"

	"github.com/go-chi/chi/v5"
)

func (s *server) handleGetSensorData(w http.ResponseWriter, r *http.Request) {
	sensorID, err := strconv.ParseInt(chi.URLParam(r, "sensorID"), 10, 64)
	if err != nil {
		s.respondErr(w, err, 400)
		return
	}
	data, err := sensordata.GetBySensorID(s.db, int(sensorID))
	if err != nil {
		s.respondErr(w, err, 500)
		return
	}
	s.respondJSON(w, data)
}

func (s *server) handleIndex(w http.ResponseWriter, r *http.Request) {
	s.respondJSON(w, map[string]string{"message": "Welcome to Smart Crops!"})
}
