from flask import request, jsonify
from app import app, app_logger
from app.db import db
from app.models.manufactory import Manufactory

logger = app_logger.get_logger(__name__)


@app.route('/update', methods=["PUT"])
def update():
    response = None
    try:
        logger.info('Handling request: Updating a record in a table')
        manufactory_id = request.args.get('id')
        manufactory_name = request.args.get('name')
        manufactory_boss = request.args.get('boss')
        manufactory_workers_count = request.args.get('workers_count')
        manufactory_company_name = request.args.get('company_name')
        manufactory = db.session.query(Manufactory).get(manufactory_id)
        manufactory.name = manufactory_name
        manufactory.boss = manufactory_boss
        manufactory.workers_count = manufactory_workers_count
        manufactory.company_name = manufactory_company_name
        db.session.commit()
        response = jsonify({'success': 'Record successfully updated.'})
        response.status_code = 200
    except Exception as e:
        db.session.rollback()
        response = jsonify({'error': 'The request had invalid values.'})
        response.status_code = 400
        logger.error(str(e))
    finally:
        return response
