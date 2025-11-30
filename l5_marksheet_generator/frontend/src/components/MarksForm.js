// client/src/components/MarksForm.js
import React, { useState } from 'react';
import { Form, Button, Card } from 'react-bootstrap';
import axios from 'axios';

const initial = {
  roll_no: '',
  first_name: '',
  last_name: '',
  subject1_mse: '',
  subject1_ese: '',
  subject2_mse: '',
  subject2_ese: '',
  subject3_mse: '',
  subject3_ese: '',
  subject4_mse: '',
  subject4_ese: ''
};

export default function MarksForm() {
  const [form, setForm] = useState(initial);
  const [msg, setMsg] = useState(null);

  const handleChange = (e) => setForm({ ...form, [e.target.name]: e.target.value });

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      const res = await axios.post('http://localhost:4000/api/marks', form);
      setMsg(`Saved. Total: ${res.data.total}, Grade: ${res.data.grade}`);
      setForm(initial);
    } catch (err) {
      console.error(err);
      setMsg('Error saving');
    }
  };

  return (
    <Card className="mb-3">
      <Card.Body>
        <Card.Title>Enter Details</Card.Title>
        <Form onSubmit={handleSubmit}>
          <Form.Group className="mb-2">
            <Form.Label>Roll No.</Form.Label>
            <Form.Control name="roll_no" placeholder="Enter Roll No." value={form.roll_no} onChange={handleChange} required />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>First Name</Form.Label>
            <Form.Control name="first_name" placeholder="First Name" value={form.first_name} onChange={handleChange} />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>Last Name</Form.Label>
            <Form.Control name="last_name" placeholder="Last Name" value={form.last_name} onChange={handleChange} />
          </Form.Group>

          <hr />
          <h6>Subject 1</h6>
          <Form.Group className="mb-2">
            <Form.Label>MSE (out of 30)</Form.Label>
            <Form.Control name="subject1_mse" value={form.subject1_mse} onChange={handleChange} />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>ESE (out of 70)</Form.Label>
            <Form.Control name="subject1_ese" value={form.subject1_ese} onChange={handleChange} />
          </Form.Group>

          {/* Subject 2 */}
          <h6>Subject 2</h6>
          <Form.Group className="mb-2">
            <Form.Label>MSE</Form.Label>
            <Form.Control name="subject2_mse" value={form.subject2_mse} onChange={handleChange} />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>ESE</Form.Label>
            <Form.Control name="subject2_ese" value={form.subject2_ese} onChange={handleChange} />
          </Form.Group>

          {/* Subject 3 */}
          <h6>Subject 3</h6>
          <Form.Group className="mb-2">
            <Form.Label>MSE</Form.Label>
            <Form.Control name="subject3_mse" value={form.subject3_mse} onChange={handleChange} />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>ESE</Form.Label>
            <Form.Control name="subject3_ese" value={form.subject3_ese} onChange={handleChange} />
          </Form.Group>

          {/* Subject 4 */}
          <h6>Subject 4</h6>
          <Form.Group className="mb-2">
            <Form.Label>MSE</Form.Label>
            <Form.Control name="subject4_mse" value={form.subject4_mse} onChange={handleChange} />
          </Form.Group>
          <Form.Group className="mb-2">
            <Form.Label>ESE</Form.Label>
            <Form.Control name="subject4_ese" value={form.subject4_ese} onChange={handleChange} />
          </Form.Group>

          <div className="d-flex justify-content-between mt-3">
            <Button variant="success" type="submit">Add Marks</Button>
            <Button variant="warning" type="button" onClick={() => setForm(initial)}>Clear</Button>
          </div>
          {msg && <p className="mt-2">{msg}</p>}
        </Form>
      </Card.Body>
    </Card>
  );
}
