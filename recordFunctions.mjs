import fs from 'fs';

export function clearRecord(){
  const filePath = './recording.txt';
  fs.writeFile(filePath, '', (err) => {
      if (err) {
          console.error(err);
      } else {
          console.log('File cleared successfully.');
      }
  });
}

export function recordAction(dir, time){ 
  const filePath = './recording.txt';
  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
      
    const updatedContent = data + `${dir}|${time}\n`;
    
    fs.writeFile(filePath, updatedContent, 'utf8', (err) => {
      if (err) {
        console.error(err);
        return;
      }
    });
  });
}

export function readRecord(callback) {
  const filePath = './recording.txt';
  var recordList = [];

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      callback(err, null);
      return;
    }

    const lines = data.split('\n');
    //line[0] is direction, line[1] is time
    for (const line of lines) {
      if (line.trim() === '') continue;
      const [direction, time] = line.split('|');
      console.log(`Reading Direction: ${direction}`);
      recordList.push({ direction, time });
    }

    // Invoke the callback with the populated recordList
    callback(null, recordList);
  });
}




