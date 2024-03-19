# Querying the Plate Reader

```bash
curl -u ditto:ditto -X POST -H 'Content-Type: application/json' -d '{
    "title": "My Plate Reader",
    "description": "ALPR device",  
    "definition": "@file:./plate-reader.jsonld",
    "attributes": {
        "deviceID": "plate_reader_1234"
    }
}' 'http://localhost:8080/api/2/things'
```
