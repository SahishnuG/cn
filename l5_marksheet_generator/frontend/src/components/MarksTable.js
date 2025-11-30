// client/src/components/MarksTable.js
import React, { useEffect, useState } from 'react';
import { Table, Button, InputGroup, FormControl } from 'react-bootstrap';
import axios from 'axios';

export default function MarksTable() {
  const [rows, setRows] = useState([]);
  const [q, setQ] = useState('');

  const load = async () => {
    const res = await axios.get('http://localhost:4000/api/marks');
    setRows(res.data);
  };
  useEffect(() => { load(); }, []);

  const del = async (id) => {
    if (!window.confirm('Delete this record?')) return;
    await axios.delete(`http://localhost:4000/api/marks/${id}`);
    load();
  };

  const search = async () => {
    if (!q) { load(); return; }
    const res = await axios.get(`http://localhost:4000/api/search?roll=${encodeURIComponent(q)}`);
    setRows(res.data);
  };

  return (
    <>
      <InputGroup className="mb-3">
        <FormControl placeholder="Search by roll_no" value={q} onChange={e => setQ(e.target.value)} />
        <Button variant="primary" onClick={search}>Search</Button>
        <Button variant="secondary" onClick={load}>Refresh</Button>
      </InputGroup>

      <Table bordered hover responsive>
        <thead>
          <tr>
            <th>Roll</th><th>Name</th><th>Sub1</th><th>Sub2</th><th>Sub3</th><th>Sub4</th><th>Total</th><th>Grade</th><th>Action</th>
          </tr>
        </thead>
        <tbody>
          {rows.map(r => (
            <tr key={r.id}>
              <td>{r.roll_no}</td>
              <td>{(r.first_name||'') + ' ' + (r.last_name||'')}</td>
              <td>{r.subject1_mse}/{r.subject1_ese}</td>
              <td>{r.subject2_mse}/{r.subject2_ese}</td>
              <td>{r.subject3_mse}/{r.subject3_ese}</td>
              <td>{r.subject4_mse}/{r.subject4_ese}</td>
              <td>{r.total}</td>
              <td>{r.grade}</td>
              <td><Button size="sm" variant="danger" onClick={()=>del(r.id)}>Delete</Button></td>
            </tr>
          ))}
        </tbody>
      </Table>
    </>
  );
}
