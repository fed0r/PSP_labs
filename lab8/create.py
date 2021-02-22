from flask import request, jsonify
from app import app, app_logger
from app.db import db
from app.models.manufactory import Manufactory

logger = app_logger.get_logger(__name__)


@app.route('/create', methods=["POST"])
def create():
    response = None
    try:
        logger.info('Handling request: Adding a record to a table')
        manufactory_name = request.args.get('name')
        manufactory_boss = request.args.get('boss')
        manufactory_workers_count = request.args.get('workers_count')
        manufactory_company_name = request.args.get('company_name')
        manufactory = Manufactory(name=manufactory_name, boss=manufactory_boss,
                                  workers_count=manufactory_workers_count,
                                  company_name=manufactory_company_name)
        db.session.add(manufactory)
        db.session.commit()
        response = jsonify({'success': 'Record added successfully.'})
        response.status_code = 200
    except Exception as e:
        db.session.rollback()
        response = jsonify({'error': 'The request had invalid values.'})
        response.status_code = 400
        logger.error(str(e))
    finally:
        return response
