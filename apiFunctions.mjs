import fs from 'fs';

export function toggleLight(callback) {
  const filePath = './apiData.txt';

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      callback(err);
      return;
    }
      
    const lines = data.split('\n');
    let firstLine = lines[0];
    const parts = firstLine.split(':');
    const textAfterColon = parts[1].trim();
    
    let ledStatus;
    if (textAfterColon === "0") {
      ledStatus = "1";
    } else {
      ledStatus = "0";
    }

    console.log(`testing ledStatus: ${ledStatus}`);
    
    lines[0] = `ledStatus:${ledStatus}`;
    
    const updatedContent = lines.join('\n');
    
    fs.writeFile(filePath, updatedContent, 'utf8', (err) => {
      if (err) {
        console.error(err);
        callback(err);
        return;
      }

      callback(null, ledStatus);
    });
  });
}

export function getLightStatus() {

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
    ledStatus = parts[1].trim();
    let returnString = 'ledStatus: ' + ledStatus;
    callback(null, returnString);
  });
}