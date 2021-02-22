from app.db import db


class Manufactory(db.Model):
    __tablename__ = 'manufactory'
    id=db.Column('id', db.Integer, primary_key=True)
    name=db.Column('name', db.String(32))
    boss=db.Column('boss', db.String(32))
    workers_count=db.Column('workers_count', db.Integer)
    company_name=db.Column('company_name', db.String(32))

    @property
    def serialize(self):
        return {'manufactory': {'id':  self.id, 'name': self.name, 'boss': self.boss, 'workers_count': self.workers_count,
                         'company_name': self.company_name}}

    @property
    def serialize_many2many(self):
        return [item.serialize for item in self.many2many]


