// server/server.js
const express = require('express');
const pool = require('./db');
const cors = require('cors');
require('dotenv').config();

const app = express();
app.use(cors());
app.use(express.json());

// helper: compute total and grade
function computeTotalAndGrade(marks) {
  // each subject: total_sub = 0.3*MSE + 0.7*ESE (assuming MSE out of 30, ESE out of 70)
  const s = (mse, ese) => (0.3 * mse) + (0.7 * ese);
  const sub1 = s(marks.subject1_mse, marks.subject1_ese);
  const sub2 = s(marks.subject2_mse, marks.subject2_ese);
  const sub3 = s(marks.subject3_mse, marks.subject3_ese);
  const sub4 = s(marks.subject4_mse, marks.subject4_ese);
  const total = sub1 + sub2 + sub3 + sub4;
  // example grade mapping (modify as you prefer)
  const percent = (total / 400) * 100; // if each subject max 100
  let grade = 'F';
  if (percent >= 85) grade = 'A+';
  else if (percent >= 75) grade = 'A';
  else if (percent >= 65) grade = 'B';
  else if (percent >= 50) grade = 'C';
  else grade = 'F';
  return { total: parseFloat(total.toFixed(2)), grade, percent: parseFloat(percent.toFixed(2)) };
}

// Create student + marks (if student exists, use them)
app.post('/api/marks', async (req, res) => {
  try {
    const body = req.body;
    // body: roll_no, first_name, last_name, subject1_mse, subject1_ese, ...
    const conn = await pool.getConnection();
    try {
      // ensure student exists
      let [rows] = await conn.query('SELECT id FROM students WHERE roll_no = ?', [body.roll_no]);
      let studentId;
      if (rows.length === 0) {
        const [r] = await conn.query('INSERT INTO students (roll_no, first_name, last_name) VALUES (?, ?, ?)', [body.roll_no, body.first_name || null, body.last_name || null]);
        studentId = r.insertId;
      } else {
        studentId = rows[0].id;
      }

      const marks = {
        subject1_mse: Number(body.subject1_mse || 0),
        subject1_ese: Number(body.subject1_ese || 0),
        subject2_mse: Number(body.subject2_mse || 0),
        subject2_ese: Number(body.subject2_ese || 0),
        subject3_mse: Number(body.subject3_mse || 0),
        subject3_ese: Number(body.subject3_ese || 0),
        subject4_mse: Number(body.subject4_mse || 0),
        subject4_ese: Number(body.subject4_ese || 0),
      };
      const cg = computeTotalAndGrade(marks);
      const [ins] = await conn.query(
        `INSERT INTO marks
         (student_id, subject1_mse, subject1_ese, subject2_mse, subject2_ese, subject3_mse, subject3_ese, subject4_mse, subject4_ese, total, grade)
         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
        [studentId, marks.subject1_mse, marks.subject1_ese, marks.subject2_mse, marks.subject2_ese, marks.subject3_mse, marks.subject3_ese, marks.subject4_mse, marks.subject4_ese, cg.total, cg.grade]
      );
      res.json({ id: ins.insertId, total: cg.total, grade: cg.grade, percent: cg.percent });
    } finally {
      conn.release();
    }
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: 'internal' });
  }
});

// Get all marks (with student info)
app.get('/api/marks', async (req, res) => {
  try {
    const [rows] = await pool.query(
      `SELECT m.*, s.roll_no, s.first_name, s.last_name
       FROM marks m JOIN students s ON m.student_id = s.id
       ORDER BY m.updated_at DESC`);
    res.json(rows);
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: 'internal' });
  }
});

// Get one
app.get('/api/marks/:id', async (req, res) => {
  try {
    const [rows] = await pool.query(
      `SELECT m.*, s.roll_no, s.first_name, s.last_name
      FROM marks m JOIN students s ON m.student_id = s.id WHERE m.id = ?`, [req.params.id]);
    if (rows.length === 0) return res.status(404).json({ error: 'not found' });
    res.json(rows[0]);
  } catch (err) { console.error(err); res.status(500).json({ error: 'internal' }); }
});

// Update marks entry
app.put('/api/marks/:id', async (req, res) => {
  try {
    const body = req.body;
    const marks = {
      subject1_mse: Number(body.subject1_mse || 0),
      subject1_ese: Number(body.subject1_ese || 0),
      subject2_mse: Number(body.subject2_mse || 0),
      subject2_ese: Number(body.subject2_ese || 0),
      subject3_mse: Number(body.subject3_mse || 0),
      subject3_ese: Number(body.subject3_ese || 0),
      subject4_mse: Number(body.subject4_mse || 0),
      subject4_ese: Number(body.subject4_ese || 0),
    };
    const cg = computeTotalAndGrade(marks);
    await pool.query(
      `UPDATE marks SET subject1_mse=?, subject1_ese=?, subject2_mse=?, subject2_ese=?, subject3_mse=?, subject3_ese=?, subject4_mse=?, subject4_ese=?, total=?, grade=? WHERE id=?`,
      [marks.subject1_mse, marks.subject1_ese, marks.subject2_mse, marks.subject2_ese, marks.subject3_mse, marks.subject3_ese, marks.subject4_mse, marks.subject4_ese, cg.total, cg.grade, req.params.id]
    );
    res.json({ id: req.params.id, total: cg.total, grade: cg.grade, percent: cg.percent });
  } catch (err) { console.error(err); res.status(500).json({ error: 'internal' }); }
});

// Delete
app.delete('/api/marks/:id', async (req, res) => {
  try {
    await pool.query('DELETE FROM marks WHERE id=?', [req.params.id]);
    await pool.query('DELETE FROM students WHERE id=?', [req.params.id]);
    res.json({ ok: true });
  } catch (err) { console.error(err); res.status(500).json({ error: 'internal' }); }
});

// Search by roll_no (query param ?roll=VT2025001)
app.get('/api/search', async (req, res) => {
  try {
    const roll = req.query.roll;
    if (!roll) return res.json([]);
    const [rows] = await pool.query(
      `SELECT m.*, s.roll_no, s.first_name, s.last_name
       FROM marks m JOIN students s ON m.student_id = s.id WHERE s.roll_no LIKE ?`, [`%${roll}%`]);
    res.json(rows);
  } catch (err) { console.error(err); res.status(500).json({ error: 'internal' }); }
});

const port = process.env.PORT || 4000;
app.listen(port, () => console.log('Server listening on', port));
