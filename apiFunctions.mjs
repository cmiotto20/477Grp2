import fs from 'fs';

export function toggleRow(row, callback) {
  const filePath = './apiData.txt';

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      callback(err);
      return;
    }
      
    const lines = data.split('\n');
    let firstLine = lines[row];
    const parts = firstLine.split(':');
    const textAfterColon = parts[1].trim();
    
    let valStatus;
    if (textAfterColon === "0") {
      valStatus = "1";
    } else {
      valStatus = "0";
    }

    console.log(`testing ${parts[0]}: ${valStatus}`);
    
    lines[row] = `${parts[0]}:${valStatus}`;
    
    const updatedContent = lines.join('\n');
    
    fs.writeFile(filePath, updatedContent, 'utf8', (err) => {
      if (err) {
        console.error(err);
        callback(err);
        return;
      }

      callback(null, valStatus);
    });
  });
}

export function getRowStatus(row, callback) {

  const filePath = './apiData.txt';
  let valStatus;

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
  
    // Split the content into lines
    const lines = data.split('\n');

    let firstLine = lines[row];

    // Split the string by colon
    const parts = firstLine.split(':');

    // Get the text after the colon (index 1)
    valStatus = parts[1].trim();
    let returnString = valStatus;
    callback(null, returnString);
  });
}