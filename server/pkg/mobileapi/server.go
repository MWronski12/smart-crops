package mobileapi

import (
	"encoding/json"
	"net/http"

	"github.com/go-chi/chi/v5"
	"gorm.io/gorm"
)

type server struct {
	db *gorm.DB
	r  chi.Router
}

func Run(db *gorm.DB, port string) error {
	s := &server{
		db: db,
		r:  chi.NewRouter(),
	}
	s.routes()
	return http.ListenAndServe(":"+port, s.r)
}

func (s *server) respondJSON(w http.ResponseWriter, data interface{}) {
	b, err := json.Marshal(data)
	if err != nil {
		s.respondErr(w, err, 500)
		return
	}
	w.Write(b)
}

func (s *server) respondErr(w http.ResponseWriter, err error, code int) {
	type Response struct {
		Code  int    `json:"code"`
		Error string `json:"error"`
	}
	r := Response{code, err.Error()}
	s.respondJSON(w, r)
}
