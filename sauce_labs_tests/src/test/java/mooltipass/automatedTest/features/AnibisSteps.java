package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Anibis;;

public class AnibisSteps {

	Anibis anibis= new Anibis(WebDriverFactory.get());

	@When("I login Anibis with '(.*)'")
	public void login(String username){
		anibis.goToLogin();
		anibis.enterEmail(username);
		String password =System.getenv().get("ANIBISPASS");
		anibis.enterPassword(password);
		anibis.submit();
		
	}
	@When("I logout Anibis")
	public void pressLogout(){
		anibis.logout();
		
	}
	@When("I go to Anibis login page")
	public void pressLogin(){
		anibis.goToLogin();
		Assert.assertTrue("Expected to be at login page", anibis.checkAtLoginPage());
		
	}
	
	@Then("I should be logged in Anibis")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",anibis.checkLogin());
	}
}
