package mqtt

import (
	"fmt"
	"time"

	paho "github.com/eclipse/paho.mqtt.golang"
)

const (
	brokerUrl = "tcp://172.111.242.63:6666"
	username  = "roslina"
	password  = "smartcrops"
	timeout   = time.Second * 5
)

/* ------------------------------- Public API ------------------------------- */

type Msg = paho.Message
type CallbackFunc = func(Msg)
type Client interface {
	Pub(topic string, qos byte, retained bool, payload interface{}) error
	Sub(topic string, qos byte, cb CallbackFunc) error
}

func Connect(url, username, password string) (Client, error) {
	// Create options
	opts := paho.NewClientOptions()
	opts.AddBroker(url)
	opts.Username = username
	opts.Password = password

	// Create a client
	c := internalClient{
		pahoClient: paho.NewClient(opts),
	}

	// Connect
	t := c.pahoClient.Connect()
	if err := waitForPahoToken(t); err != nil {
		return nil, fmt.Errorf("failed to connect to mqtt broker: %w", err)
	}
	return &c, nil
}

/* -------------------------------- Internals ------------------------------- */

type internalClient struct {
	pahoClient paho.Client
}

func waitForPahoToken(t paho.Token) error {
	finished := t.WaitTimeout(timeout)
	if !finished {
		return fmt.Errorf("mqtt timeout")
	}
	if t.Error() != nil {
		return t.Error()
	}
	return nil
}

func (c *internalClient) Pub(topic string, qos byte, retained bool, payload interface{}) error {
	t := c.pahoClient.Publish(topic, qos, retained, payload)
	if err := waitForPahoToken(t); err != nil {
		return fmt.Errorf("failed to publish on topic \"%s\" caused by: %w", topic, err)
	}
	return nil
}

func (c *internalClient) Sub(topic string, qos byte, cb CallbackFunc) error {
	t := c.pahoClient.Subscribe(topic, qos, func(c paho.Client, m paho.Message) { cb(m) })
	if err := waitForPahoToken(t); err != nil {
		return fmt.Errorf("failed to subscribe to topic \"%s\" caused by: %w", topic, err)
	}
	return nil
}
