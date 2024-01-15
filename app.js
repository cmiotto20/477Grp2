const express = require('express');
const cors = require('cors');
const app = express();
const port = 8080;

const fs = require('fs');

// Use CORS middleware
app.use(cors());

app.get('/expectSuccess', (req, res) => {
  res.status(200).send('This API call was a success (as expected)');
});

app.get('/expectFail', (req, res) => {
  res.status(501).send('This API call was a fail (as expected)');
});

app.post('/toggleLight', (req, res) => {

  const filePath = './apiData.txt';
  let ledStatus;

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
  
    // Split the content into lines
    const lines = data.split('\n');

    let firstLine = lines[0];

    // Split the string by colon
    const parts = firstLine.split(':');

    // Get the text after the colon (index 1)
    const textAfterColon = parts[1].trim();

    if(textAfterColon == "0") {
      ledStatus = "1"
    } else {
      ledStatus = "0";
    }
  
    // Change the first line (index 0)
    lines[0] = `ledStatus:${ledStatus}`;
  
    // Join the lines back into a string
    const updatedContent = lines.join('\n');
  
    // Write the updated content back to the file
    fs.writeFile(filePath, updatedContent, 'utf8', (err) => {
      if (err) {
        console.error(err);
        res.status(501).send(err);
      }
      });
  i});

  res.status(200).send('Light toggled');
});

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
