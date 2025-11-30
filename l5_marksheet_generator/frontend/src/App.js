// client/src/App.js
import React from 'react';
import { Container, Row, Col, Card } from 'react-bootstrap';
import MarksForm from './components/MarksForm';
import MarksTable from './components/MarksTable';

function App() {
  return (
    <Container fluid className="p-4">
      <h2 className="text-center mb-4">Marksheet Generation</h2>
      <Row>
        <Col md={4}>
          <Card className="mb-3">
            <Card.Body>
              <Card.Title>Menu</Card.Title>
              <p className="text-muted">Use the form to Add / Update marks</p>
            </Card.Body>
          </Card>
          <MarksForm />
        </Col>
        <Col md={8}>
          <Card>
            <Card.Body>
              <Card.Title>All Marksheets</Card.Title>
              <MarksTable />
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
}

export default App;
