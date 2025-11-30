-- schema.sql
CREATE DATABASE IF NOT EXISTS vit_result;
USE vit_result;

CREATE TABLE IF NOT EXISTS students (
  id INT AUTO_INCREMENT PRIMARY KEY,
  roll_no VARCHAR(50) NOT NULL UNIQUE,
  first_name VARCHAR(100),
  last_name VARCHAR(100),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS marks (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_id INT NOT NULL,
  subject1_mse INT DEFAULT 0,
  subject1_ese INT DEFAULT 0,
  subject2_mse INT DEFAULT 0,
  subject2_ese INT DEFAULT 0,
  subject3_mse INT DEFAULT 0,
  subject3_ese INT DEFAULT 0,
  subject4_mse INT DEFAULT 0,
  subject4_ese INT DEFAULT 0,
  total FLOAT DEFAULT 0,      -- computed on insert/update
  grade VARCHAR(5),
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE
);

-- sample data (optional)
INSERT INTO students (roll_no, first_name, last_name) VALUES
('VT2025001', 'Alice', 'Patil'),
('VT2025002', 'Bob', 'Kumar');

INSERT INTO marks (student_id, subject1_mse, subject1_ese, subject2_mse, subject2_ese,
                   subject3_mse, subject3_ese, subject4_mse, subject4_ese, total, grade)
VALUES
(1, 24, 56, 20, 48, 22, 60, 26, 58, 0, NULL),
(2, 18, 35, 25, 50, 15, 40, 20, 44, 0, NULL);
