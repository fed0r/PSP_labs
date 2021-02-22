from flask import jsonify
from app import app, app_logger
from app.db import db
from app.models.manufactory import Manufactory

logger = app_logger.get_logger(__name__)

@app.route('/get_all', methods=["GET"])
def get_all():
    try:
        logger.info('Handling request: Getting a list of records')
        query_list = db.session.query(Manufactory)
        response = jsonify([i.serialize for i in query_list.all()])
        return response
    except Exception as e:
        logger.error(str(e))