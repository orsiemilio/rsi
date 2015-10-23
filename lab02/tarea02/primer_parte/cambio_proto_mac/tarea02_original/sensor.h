typedef struct sensor_s{
	uint16_t valor;
	char unidades[2];
	uint16_t time_stamp;
}sensor_t;

// lab02
typedef struct pkg_s{
	sensor_t temp;
	unsigned char sender_node_id;
	unsigned char mssg_number;
}pkg_t;

