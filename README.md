# El Cienco Arsenal

## API Endpoints

### Start Attack
`POST /attack`
```json
{
    "target": "192.168.1.1",
    "port": 80,
    "threads": 100,
    "duration": 300,
    "method": 7
}
